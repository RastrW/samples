#include "mainwindow.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>

#include "qastra.h"
#include "qti.h"
#include "qbarsmdp.h"
#include "mcrwnd.h"
#include "formprotocol.h"
#include "pyhlp.h"
#include "formcalcidop.h"

#include <QStatusBar>
#include <QMessageBox>
#include <DockManager.h>
#include <DockWidget.h>
#include <QMdiArea>
#include <QInputDialog>
#include <QApplication>
#include <QMdiSubWindow>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtSvgWidgets/QSvgWidget>
#endif
#include <QSvgRenderer>

#include <spdlog/spdlog.h>

#include "calculationController.h"
#include "fileManager.h"
#include "formManager.h"
#include "settingsManager.h"
#include "uiBuilder.h"
#include "cacheLog.h"

MainWindow::MainWindow()
    : QMainWindow(){

    m_workspace = new QMdiArea(this);
    setCentralWidget(m_workspace);
    
    // Подключение сигнала активации подокон
    connect(m_workspace, &QMdiArea::subWindowActivated,
            this, &MainWindow::slot_subWindowActivated);
    
    // Настройка Drag & Drop
    setAcceptDrops(true);
    
    // Заголовок окна
    setWindowTitle(tr("~qrastr~"));
    
    // Настройка Advanced Docking System
    ads::CDockManager::setConfigFlag
        (ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setConfigFlag
        (ads::CDockManager::AllTabsHaveCloseButton, true);
    m_dockManager = new ads::CDockManager(this);
    
    // Подключение обработчика фокуса доков
    QObject::connect(m_dockManager, &ads::CDockManager::focusedDockWidgetChanged,
                     [](ads::CDockWidget* old, ads::CDockWidget* now) {
                         if (now) {
                            static int Count = 0;
                            qDebug() << Count++
                                     << " CDockManager::focusedDockWidgetChanged old: "
                                     << (old ? old->objectName() : "-")
                                     << " now: " << now->objectName()
                                     << " visible: " << now->isVisible();
                             now->widget()->setFocus();
                         }
                     });
}

MainWindow::~MainWindow() = default;

void MainWindow::initialize(
    std::shared_ptr<QAstra> qastra,
    std::shared_ptr<QTI> qti,
    std::shared_ptr<QBarsMDP> qbarsmdp,
    const std::list<CUIForm>& forms) {
    // Сохранение плагинов
    m_qastra = qastra;
    m_qti = qti;
    m_qbarsmdp = qbarsmdp;
    
    // ========== СОЗДАНИЕ КОМПОНЕНТОВ ==========
    // 1. SettingsManager (первым - нужен для загрузки настроек)
    m_settingsManager = std::make_unique<SettingsManager>(this);
    m_settingsManager->loadWindowGeometry(this);
    // Restore the state of toolbars and dock widgets (menus are part of the overall layout)
    restoreState(m_settingsManager->getSettings("mainWindowState"));

    // 2. FileManager
    m_fileManager = std::make_unique<FileManager>(qastra, this);
    
    // 3. CalculationController
    m_calcController = std::make_unique<CalculationController>(
        qastra, qti, qbarsmdp, this);
    
    // 4. FormManager
    m_rtdm.setQAstra(qastra.get());
    m_formManager = std::make_unique<FormManager>(
        qastra, &m_rtdm, m_dockManager, this);
    m_formManager->setForms(forms);
    
    // 5. UIBuilder
    m_uiBuilder = std::make_unique<UIBuilder>(this);
    m_uiBuilder->buildAll();
    
    // 6. Python helper
    m_pyHelper = std::make_unique<PyHlp>(*qastra->getRastr().get());
    
    // ========== НАСТРОЙКА КОМПОНЕНТОВ ==========
    setupDockWidgets();
    setupLogging();
    setupConnections();
    slot_updateRecentFiles();
    
    // Построение меню форм
    m_formManager->buildFormsMenu(
        m_uiBuilder->openMenu(),
        m_uiBuilder->calcParametersMenu()
    );
    m_formManager->buildPropertiesMenu(m_uiBuilder->propertiesMenu());
    
    // Вывод кэшированных логов
    m_settingsManager->flushLogCache();
    
    spdlog::info("MainWindow initialized successfully");
}

void MainWindow::setupDockWidgets() {
    // Окно макросов
    m_globalProtocol = new McrWnd(this, McrWnd::_en_role::global_protocol);
    
    auto dockProtocol = new ads::CDockWidget("protocol", this);
    dockProtocol->setWidget(m_globalProtocol);
    dockProtocol->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    m_dockManager->addDockWidgetTab(ads::BottomDockWidgetArea, dockProtocol);
    
    // Главный протокол
    m_mainProtocol = new FormProtocol(this);
    m_mainProtocol->setIgnoreAppendProtocol(true);

    auto dockMainProtocol = new ads::CDockWidget("protocolMain", this);
    dockMainProtocol->setWidget(m_mainProtocol);
    dockMainProtocol->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    m_dockManager->addDockWidgetTab(ads::BottomDockWidgetArea, dockMainProtocol);
}

void MainWindow::setupLogging() {
    auto logger = spdlog::default_logger();
    
    // Sink для окна протокола (макросы)
    auto qtSink = std::make_shared<spdlog::sinks::qt_sink_mt>(
        m_globalProtocol, "onQStringAppendProtocol"
    );
    logger->sinks().push_back(qtSink);
    
    // Sink для главного протокола
    auto protocolSink = std::make_shared<spdlog::sinks::qt_sink_mt>(
        m_mainProtocol, "onAppendProtocol");
    logger->sinks().push_back(protocolSink);
    
    // Подключение логов от Rastr
    connect(m_qastra.get(), &QAstra::onRastrLog,
            m_mainProtocol, &FormProtocol::onRastrLog);
    connect(m_qastra.get(), &QAstra::onRastrLog,
            m_globalProtocol, &McrWnd::onRastrLog);
}

void MainWindow::setupConnections() {
    // ========== FILEMANAGER ==========
    // Файловые действия
    connect(m_uiBuilder->actionByName("new"), &QAction::triggered,
            m_fileManager.get(), &FileManager::newFile);
    
    connect(m_uiBuilder->actionByName("open"), &QAction::triggered,
            m_fileManager.get(), &FileManager::openFiles);
    
    connect(m_uiBuilder->actionByName("save"), &QAction::triggered,
            m_fileManager.get(), &FileManager::save);
    
    connect(m_uiBuilder->actionByName("saveAs"), &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAs);
    
    connect(m_uiBuilder->actionByName("saveAll"), &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAll);
    
    // События файлов
    connect(m_fileManager.get(), &FileManager::currentFileChanged,
            this, [this](const QString& file) {
                setWindowTitle(file);
                emit sig_fileLoaded();
            });
    connect(m_fileManager.get(), &FileManager::filesOpened,
            this, [](int count) {
                spdlog::info("Opened {} files", count);
            });
    connect(m_fileManager.get(), &FileManager::fileLoadError,
            this, [](const QString& error) {
                spdlog::error("File load error: {}", error.toStdString());
            });
    connect(m_fileManager.get(), &FileManager::recentFilesChanged,
            this, &MainWindow::slot_updateRecentFiles); 
    connect(m_uiBuilder->actionByName("exit"), &QAction::triggered,
            qApp, &QApplication::closeAllWindows);

    // ========== CALCULATIONCONTROLLER ==========
    // Основные расчёты
    connect(m_uiBuilder->actionByName("kdd"), &QAction::triggered,
            m_calcController.get(), [this]() {
                m_calcController->executeKdd();
            });
    
    connect(m_uiBuilder->actionByName("rgm"), &QAction::triggered,
            m_calcController.get(), [this]() {
                m_calcController->executeRgm();
            });
    
    connect(m_uiBuilder->actionByName("opf"), &QAction::triggered,
            m_calcController.get(), [this]() {
                m_calcController->executeOPF("s");
            });
    
    connect(m_uiBuilder->actionByName("smzu"), &QAction::triggered,
            m_calcController.get(), [this]() {
                m_calcController->executeSMZUtst("33");
            });
    
    connect(m_uiBuilder->actionByName("kz"), &QAction::triggered,
            m_calcController.get(), [this]() {
                KzParameters params;
                params.parameters = "";
                params.nonsym = eNonsym::KZ_1;
                params.p1 = 1;
                params.p2 = 0;
                params.p3 = 0;
                params.lengthFromP1InProc = 0;
                params.rd = 0;
                params.z_re = 0;
                params.z_im = 0;
                m_calcController->executeTkz(params);
            });
    // Диалоговые расчёты
    connect(m_uiBuilder->actionByName("idop"), &QAction::triggered,
            m_calcController.get(), &CalculationController::showIdopDialog);
    
    connect(m_calcController.get(), &CalculationController::showDialogRequested,
            this, [this](const QString& dialogType, const QString& params) {
                if (dialogType == "idop") {
                    showIdopDialog();
                } else if (dialogType == "mdp_prepare") {
                    showMDPPrepareDialog();
                }
            });
    
    // ТИ расчёты
    connect(m_uiBuilder->actionByName("recalcDor"), &QAction::triggered,
            m_calcController.get(), &CalculationController::recalcTiDor);
    
    connect(m_uiBuilder->actionByName("updateTables"), &QAction::triggered,
            m_calcController.get(), &CalculationController::updateTiTables);
    
    connect(m_uiBuilder->actionByName("calcPTI"), &QAction::triggered,
            m_calcController.get(), &CalculationController::calcPTI);
    
    connect(m_uiBuilder->actionByName("filtrTI"), &QAction::triggered,
            m_calcController.get(), &CalculationController::filtrTI);
    
    // МДП подготовка
    connect(m_uiBuilder->actionByName("prepareMDP"), &QAction::triggered,
            m_calcController.get(), [this]() {
                m_calcController->prepareBarsMDP("");
            });
    
    // События расчётов
    connect(m_calcController.get(), &CalculationController::calculationStarted,
            this, [this](const QString& type) {
                emit sig_calcBegin();
                spdlog::info("Calculation started: {}", type.toStdString());
            });
    
    connect(m_calcController.get(), &CalculationController::calculationFinished,
            this, [this](const QString& type, bool success) {
                emit sig_calcEnd();
                spdlog::info("Calculation finished: {} (success: {})",
                    type.toStdString(), success);
            });
    
    connect(m_calcController.get(), &CalculationController::statusMessage,
            statusBar(), &QStatusBar::showMessage);
    
    // ========== FORMMANAGER ==========
    // Передача сигналов расчётов формам
    connect(this, &MainWindow::sig_calcBegin,
            m_formManager.get(), &FormManager::onCalculationStarted);
    
    connect(this, &MainWindow::sig_calcEnd,
            m_formManager.get(), &FormManager::onCalculationFinished);
    
    connect(m_formManager.get(), &FormManager::formOpened,
            this, [](const QString& name) {
                spdlog::info("Form opened: {}", name.toStdString());
            });
    
    // ========== SETTINGSMANAGER ==========
    connect(m_uiBuilder->actionByName("settings"), &QAction::triggered,
            m_settingsManager.get(), [this]() {
                m_settingsManager->showFormSettings(m_qastra);
            });

    // ========== Окна ==========
    connect(m_uiBuilder->actionByName("graph"), &QAction::triggered,
            this, &MainWindow::slot_openGraph);
    connect(m_uiBuilder->actionByName("macro"), &QAction::triggered,
            this, &MainWindow::slot_openMcrDialog);
    connect(m_uiBuilder->actionByName("close"), &QAction::triggered,
            m_workspace, &QMdiArea::closeActiveSubWindow);
    connect(m_uiBuilder->actionByName("closeAll"), &QAction::triggered,
            m_workspace, &QMdiArea::closeAllSubWindows);
    connect(m_uiBuilder->actionByName("tile"), &QAction::triggered,
            m_workspace, &QMdiArea::tileSubWindows);
    connect(m_uiBuilder->actionByName("cascade"), &QAction::triggered,
            m_workspace, &QMdiArea::cascadeSubWindows);
    connect(m_uiBuilder->actionByName("next"), &QAction::triggered,
            m_workspace, &QMdiArea::activateNextSubWindow);
    connect(m_uiBuilder->actionByName("previous"), &QAction::triggered,
            m_workspace, &QMdiArea::activatePreviousSubWindow);
    connect(m_uiBuilder->actionByName("about"), &QAction::triggered,
            this, &MainWindow::slot_about);
}

void MainWindow::slot_updateRecentFiles() {
    QStringList files = m_fileManager->getRecentFiles();
    m_uiBuilder->updateRecentFileActions(files);
}

void MainWindow::slot_openMcrDialog(){
    McrWnd* pMcrWnd = new McrWnd( this, McrWnd::_en_role::macro_dlg );
    connect(m_qastra.get(), &QAstra::onRastrPrint, pMcrWnd, &McrWnd::onRastrPrint);

    pMcrWnd->setPyHlp(m_pyHelper.get());
    pMcrWnd->show();
}

void MainWindow::slot_openGraph(){

    auto dw = new ads::CDockWidget( "Графика", this);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    //Using QSvgWidget
    QSvgWidget *svgWidget = new QSvgWidget;
    svgWidget->load(QStringLiteral(":/images/cx195.svg"));
    dw->setWidget(svgWidget);
    connect( dw, SIGNAL( closed() ), svgWidget, SLOT( OnClose() ) );
    auto area = m_dockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
#endif

#if(!defined(SDL_NO))
    SDL_Init(SDL_INIT_VIDEO); // Basics of SDL, init what you need to use
    SDLChild * SdlChild = new SDLChild(dw);	// Creating the SDL Window and initializing it.

    dw->setWidget(SdlChild);
    connect( dw, SIGNAL( closed() ), SdlChild, SLOT( OnClose() ) );
    auto area = m_dockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

    SdlChild->SDLInit();
#endif

}

void MainWindow::slot_about(){
    QMessageBox::about(this, tr("About QRastr"),
                       tr("About the <b>QRastr</b>.") );
}

void MainWindow::showIdopDialog() {  
    emit sig_calcBegin();
    
    formcalcidop* dialog = new formcalcidop(m_qastra.get(), this);
    dialog->show();

    emit sig_calcEnd();
}

void MainWindow::showMDPPrepareDialog() {

    bool ok;
    QString text = QInputDialog::getText(
        this,
        tr("Подготовка для расчета МДП"),
        tr("Сечения:"),
        QLineEdit::Normal,
        "0",
        &ok);
    
    if (ok && !text.isEmpty()) {
        m_calcController->prepareBarsMDP(text);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Сохранение настроек
    m_settingsManager->saveWindowGeometry(this);
    
    spdlog::info("MainWindow closing");
    
    // Удаление dock manager (иначе незакреплённые окна не закроются)
    if (m_dockManager) {
        m_dockManager->deleteLater();
    }
    
    QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
    for (const QUrl& url : event->mimeData()->urls()) {
        QString filePath = url.toLocalFile();
        m_fileManager->openFile(filePath);
    }
    
    event->acceptProposedAction();
}

void MainWindow::slot_subWindowActivated() {
}

void MainWindow::slot_updateMenu() {
    // Обновление меню
}
