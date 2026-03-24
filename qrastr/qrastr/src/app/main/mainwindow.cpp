#include "mainwindow.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>

#include "qastra.h"
#include "qti.h"
#include "qbarsmdp.h"
#include "mcrwnd.h"
#include "qmcr/pyhlp.h"
#include "calcIacceptableDialog.h"

#include <QStatusBar>
#include <QMessageBox>
#include <DockManager.h>
#include <DockWidget.h>
#include <QMdiArea>
#include <QInputDialog>
#include <QApplication>
#include <QMdiSubWindow>

#include <QTimer>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QtSvgWidgets/QSvgWidget>
#include <QSvgRenderer>
#endif

#include <spdlog/spdlog.h>

#include "calculationController.h"
#include "fileManager.h"
#include "formManager.h"
#include "appSettingsManager.h"
#include "uiBuilder.h"
#include "params.h"
#include "UIForms.h"
#include "workspaceManager.h"

MainWindow::MainWindow()
    : QMainWindow(){

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

    // Создаём виджеты протоколов и СРАЗУ добавляем Qt-синки в логгер.
    m_logManager = new LogManager(m_dockManager, this);
    m_logManager->createWidgets();
    m_logManager->setupLogSinks();
}

MainWindow::~MainWindow() = default;

void MainWindow::initialize(
    std::shared_ptr<QAstra> qastra,
    std::shared_ptr<QTI> qti,
    std::shared_ptr<QBarsMDP> qbarsmdp,
    const std::list<CUIForm>& forms)
{
    m_qastra   = qastra;
    m_qti      = qti;
    m_qbarsmdp = qbarsmdp;

    // ========== СОЗДАНИЕ КОМПОНЕНТОВ ==========
    // SettingsManager нужен первым для загрузки настроек
    m_appSettingsManager = std::make_unique<AppSettingsManager>(this);
    m_appSettingsManager->loadWindowGeometry(this);
    // Restore the state of toolbars and dock widgets (menus are part of the overall layout)
    restoreState(m_appSettingsManager->getSettings("mainWindowState"));

    m_fileManager = std::make_unique<FileManager>(m_qastra, this);

    const auto& startFiles = Params::get_instance()->getStartLoadFileTemplates();
    for (const auto& [file, tmpl] : startFiles) {
        // Добавляем в карту БЕЗ добавления в "последние"
        m_fileManager->registerStartupFile(
            QString::fromStdString(file),
            QString::fromStdString(tmpl));
    }

    m_calcController = std::make_unique<CalculationController>(
        m_qastra, m_qti, m_qbarsmdp, this);
    m_pyHelper    = std::make_shared<PyHlp>(*m_qastra->getRastr().get());
    m_formManager = std::make_unique<FormManager>(
        m_qastra, m_dockManager, m_pyHelper, m_logManager, this);
    m_formManager->setForms(forms);
    m_uiBuilder = std::make_unique<UIBuilder>(this);
    m_uiBuilder->buildAll();
    // ========== НАСТРОЙКА КОМПОНЕНТОВ ==========
    m_logManager->setupRastrConnections(m_qastra);
    setupConnections();
    slot_updateRecentFiles();
    // Построение меню форм
    m_formManager->buildFormsMenu(
        m_uiBuilder->tablesMenu(),
        m_uiBuilder->calcParametersMenu());
    m_formManager->buildPropertiesMenu(m_uiBuilder->propertiesMenu());

    m_workspaceManager = std::make_unique<WorkspaceManager>(
        m_appSettingsManager->settings(),
        m_dockManager,
        m_formManager.get(),
        this);
    m_workspaceManager->applyStartupWorkspace();

    // Создаём виджеты протоколов и СРАЗУ добавляем Qt-синки в логгер.
    m_logManager->setupDockWidgets();

    spdlog::info("MainWindow initialized successfully");
}

void MainWindow::setupConnections() {
    // ========== FILEMANAGER ==========
    // Файловые действия
    connect(m_uiBuilder->actionByName("new"), &QAction::triggered,
            m_fileManager.get(), &FileManager::newFile);

    connect(m_uiBuilder->actionByName("load"), &QAction::triggered,
            m_fileManager.get(), &FileManager::openFiles);

    connect(m_uiBuilder->actionByName("save"), &QAction::triggered,
            m_fileManager.get(), &FileManager::save);

    connect(m_uiBuilder->actionByName("saveAs"), &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAs);

    connect(m_uiBuilder->actionByName("saveAll"), &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAll);

    for (int i = 0; i < Params::get_instance()->getMaxRecentFiles(); ++i) {
        QString name = QString("recentFile%1").arg(i);
        QAction* act = m_uiBuilder->actionByName(name);
        if (act) {
            connect(act, &QAction::triggered, this, [this, act]() {
                m_fileManager->openRecentFile(act->data().toString());
            });
        }
    }
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
            m_formManager.get(), &FormManager::slot_calculationStarted);

    connect(this, &MainWindow::sig_calcEnd,
            m_formManager.get(), &FormManager::slot_calculationFinished);

    connect(m_formManager.get(), &FormManager::formOpened,
            this, [](const QString& name) {
                spdlog::info("Form opened: {}", name.toStdString());
            });
    connect(m_uiBuilder->actionByName("cascade"), &QAction::triggered,
            m_formManager.get(), &FormManager::slot_cascadeForms);
    connect(m_uiBuilder->actionByName("tile"), &QAction::triggered,
            m_formManager.get(), &FormManager::slot_tileForms);
    // ========== SETTINGSMANAGER ==========
    connect(m_uiBuilder->actionByName("settings"), &QAction::triggered,
            m_appSettingsManager.get(), [this]() {
                m_appSettingsManager->showFormSettings(m_qastra);
            });

    // ========== Окна ==========
    connect(m_uiBuilder->actionByName("graphWeb"), &QAction::triggered,
            m_formManager.get(), &FormManager::slot_openWebGraph);
    connect(m_uiBuilder->actionByName("graphSDL"), &QAction::triggered,
            m_formManager.get(), &FormManager::slot_openSDLGraph);
    connect(m_uiBuilder->actionByName("macro"), &QAction::triggered,
            this, &MainWindow::slot_openMcrDialog);
    connect(m_uiBuilder->actionByName("protocol"), &QAction::triggered,
            m_formManager.get(), &FormManager::slot_openProtocol);
    // Закрыть активный dock widget
    connect(m_uiBuilder->actionByName("close"), &QAction::triggered,
            this, [this]() {
                auto* focused = m_dockManager->focusedDockWidget();
                if (focused) focused->closeDockWidget();
            });

    // Закрыть все (кроме протоколов — они с CustomCloseHandling)
    connect(m_uiBuilder->actionByName("closeAll"), &QAction::triggered,
            this, [this]() {
                for (auto* dw : m_dockManager->openedDockWidgets()) {
                    if (dw->features().testFlag(
                            ads::CDockWidget::DockWidgetDeleteOnClose)) {
                        dw->closeDockWidget();
                    }
                }
            });
    connect(m_uiBuilder->actionByName("about"), &QAction::triggered,
            this, &MainWindow::slot_about);
    // ========== WORKSPACEMANAGER ==========
    connect(m_uiBuilder->actionByName("saveWorkSpace"), &QAction::triggered,
            m_workspaceManager.get(), &WorkspaceManager::slot_showSaveDialog);
    connect(m_uiBuilder->actionByName("loadWorkSpace"), &QAction::triggered,
            m_workspaceManager.get(), &WorkspaceManager::slot_showLoadDialog);
}

std::shared_ptr<spdlog::sinks::sink>
MainWindow::getProtocolLogSink() const {
    return m_logManager->getProtocolLogSink();
}


void MainWindow::slot_updateRecentFiles() {
    QStringList files = m_fileManager->getRecentFiles();
    m_uiBuilder->updateRecentFileActions(files);
}

void MainWindow::slot_openMcrDialog(){
    McrWnd* dialog = new McrWnd( this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_qastra.get(), &QAstra::onRastrPrint, dialog, &McrWnd::slot_rastrPrint);

    dialog->setPyHlp(m_pyHelper);
    dialog->show();
}

void MainWindow::slot_about(){
    QMessageBox::about(this, tr("About QRastr"),
                       tr("About the <b>QRastr</b>.") );
}

void MainWindow::showIdopDialog() {  
    emit sig_calcBegin();
    
    CalcIacceptableDialog* dialog = new CalcIacceptableDialog(m_qastra.get(), this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
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
    m_formManager->closeGraphWebServer();

    m_appSettingsManager->saveWindowGeometry(this);
    spdlog::info("MainWindow closing");

    // dockManager удаляем ПОСЛЕ того как DLL уже выгружена
    if (m_dockManager)
        m_dockManager->deleteLater();

    // сброс буфера файла до закрытия окна
    spdlog::default_logger()->flush();
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
        m_fileManager->openFile(url.toLocalFile());
    }
}

void MainWindow::slot_subWindowActivated() {
}

void MainWindow::slot_updateMenu() {
    // Обновление меню
}
