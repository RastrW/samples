#include <QtGui>
#include <QMdiArea>
#include <QFileDialog>
#include <QAction>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QDebug>
#include <QSet>
#include <QListView>
#include <QDockWidget>
#include <QAbstractTableModel>
#include <QInputDialog>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>
#include <DockManager.h>
#include "mainwindow.h"
#include "qbarsmdp.h"
#include "rtabwidget.h"
#include "qmcr/mcrwnd.h"
#include "plugins/rastr/plugin_interfaces.h"
using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>
#include "qastra.h"
#include "qti.h"
#include "tsthints.h"
#include "delegatecombobox.h"
#include "formsettings.h"
#include "formsaveall.h"
#include "rtabwidget.h"
#include "params.h"
#include "formfilenew.h"
#include "formprotocol.h"
#include "formcalcidop.h"
#include <QtitanDef.h>
#include "qmcr/pyhlp.h"
#include "fileManager.h"

MainWindow::MainWindow(){
    m_workspace = new QMdiArea (this);
    setCentralWidget(m_workspace);
    connect(m_workspace, &QMdiArea::subWindowActivated, this, &MainWindow::slot_updateMenu);

    m_windowMapper = new QSignalMapper(this);
    createActions();
    createStatusBar();
    slot_updateMenu();
    this->setAcceptDrops(true);

    setWindowTitle(tr("~qrastr~"));
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton, true);
    m_DockManager = new ads::CDockManager(this);
    QObject::connect(m_DockManager, &ads::CDockManager::focusedDockWidgetChanged
                     , [] (ads::CDockWidget* old, ads::CDockWidget* now){
                         static int Count = 0;
                         qDebug() << Count++ << " CDockManager::focusedDockWidgetChanged old: " << (old ? old->objectName() : "-") << " now: " << now->objectName() << " visible: " << now->isVisible();
                         now->widget()->setFocus();
                     });
    m_pMcrWnd = new McrWnd( this, McrWnd::_en_role::global_protocol );
    m_pFormProtocol = new FormProtocol(this);

    auto dw = new ads::CDockWidget( "protocol", this);
    dw->setWidget(m_pMcrWnd);
    int f = ads::CDockWidget::CustomCloseHandling;
    dw->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true);

    auto container = m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea,dw);

    auto pdwProtocol = new ads::CDockWidget( "protocolMain", this );
    pdwProtocol->setWidget(m_pFormProtocol);
    pdwProtocol->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true );
    auto pfdc = m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea, pdwProtocol);

    auto qt_sink = std::make_shared<spdlog::sinks::qt_sink_mt>
        (m_pMcrWnd, "onQStringAppendProtocol");
    auto logg = spdlog::default_logger();
    logg->sinks().push_back(qt_sink);

    auto qsinkProtocol = std::make_shared<spdlog::sinks::qt_sink_mt>
        (m_pFormProtocol, "onAppendProtocol");
    logg->sinks().push_back(qsinkProtocol);

    setAcceptDrops(true);
    readSettings();
}

MainWindow::~MainWindow() = default;

bool MainWindow::readSettings(){
    try{
        QSettings settings;
        settings.beginGroup("MainWindow");
        const auto geometry = settings.value("geometry", QByteArray()).toByteArray();
        if (geometry.isEmpty())
            setGeometry(200, 200, 800, 800);
        else
            restoreGeometry(geometry);
        settings.endGroup();
    }catch(const std::exception& ex){
        m_v_cache_log.add( spdlog::level::err,"Exception: {} ", ex.what());
        return false;
    }catch(...){
        m_v_cache_log.add( spdlog::level::err,"Unknown exception.");
        return false;
    }
    return true;
}

bool MainWindow::writeSettings(){
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *dropEvent){
    QStringList filePathList;
    foreach (QUrl url, dropEvent->mimeData()->urls()){
        std::string fileName = url.toLocalFile().toStdString();
        filePathList << url.toLocalFile();
        m_sp_qastra->Load( eLoadCode::RG_REPL, fileName, "" );
        setWindowTitle(tr(fileName.c_str()));
    }
    dropEvent->acceptProposedAction();
}

void MainWindow::logCacheFlush(){
    m_v_cache_log.flush();
}

void MainWindow::showEvent( QShowEvent* event ){
    QWidget::showEvent( event );
}

void MainWindow::setForms(const std::list<CUIForm>& forms){
    int i = 0;
    m_lstUIForms = forms;
    m_RTDM.setForms(&m_lstUIForms);
    QMap<QString,QMenu *> map_menu;
    for(const auto& j_form : forms){
        std::string str_MenuPath = stringutils::MkToUtf8(j_form.MenuPath());
        auto vmenu = split(str_MenuPath,'\\');
        std::string str_Name = stringutils::MkToUtf8(j_form.Name());
        if (str_MenuPath.empty())
            continue;
        if (j_form.AddToMenuIndex() >= vmenu.size() )
            continue;
        QString qstr_MenuPath = vmenu[j_form.AddToMenuIndex()].c_str();
        if (!map_menu.contains(qstr_MenuPath))
            map_menu.insert(qstr_MenuPath,m_menuOpen->addMenu(qstr_MenuPath.isEmpty()?"Остальное":qstr_MenuPath));
    }
    for(const auto& j_form : forms){
        std::string str_Name = stringutils::MkToUtf8(j_form.Name());
        std::string str_TableName = j_form.TableName();
        std::string str_MenuPath = stringutils::MkToUtf8(j_form.MenuPath());
        auto vmenu = split(str_MenuPath,'\\');
        if (!str_MenuPath.empty() && j_form.AddToMenuIndex() >= vmenu.size() ){
            i++;
            continue;
        }
        QString qstr_MenuPath;
        if (str_MenuPath.empty())
            qstr_MenuPath = "";
        else
            qstr_MenuPath = vmenu[j_form.AddToMenuIndex()].c_str();

        QMenu* cur_menu = m_menuOpen;
        if (map_menu.contains(qstr_MenuPath))
            cur_menu = map_menu[qstr_MenuPath];

        if (j_form.AddToMenuIndex() == 2)
            cur_menu = m_menuCalcParameters;

        if (!str_Name.empty() && str_Name.at(0) != '_'){
            QAction* p_actn = cur_menu->addAction(str_Name.c_str());
            p_actn->setData(i);
        }
        i++;
    }

    connect(m_menuOpen, &QMenu::triggered, this, &MainWindow::slot_openForm, Qt::UniqueConnection);
    connect(m_menuCalcParameters, &QMenu::triggered, this, &MainWindow::slot_openForm, Qt::UniqueConnection);
    connect(m_menuProperties, &QMenu::aboutToShow, this, &MainWindow::setSettingsForms);
}

void MainWindow::setSettingsForms(){
    m_menuProperties->clear();
    IRastrTablesPtr tablesx{ m_sp_qastra->getRastr()->Tables() };
    IRastrPayload cnt{tablesx->Count()};
    for (int i = 0 ; i< cnt.Value() ; i++){
        IRastrTablePtr table{ tablesx->Item(i) };
        IRastrPayload tab_name{table->Name()};
        IRastrPayload templ_name{table->TemplateName()};
        IRastrPayload tab_desc{table->Description()};
        std::string str_tab_name = tab_name.Value();
        std::string str_templ_name = templ_name.Value();

        if ( QFileInfo(str_templ_name.c_str()).suffix() == "form"){
            CUIForm _form;

            _form.SetName(tab_desc.Value().c_str());
            _form.SetTableName(tab_name.Value().c_str());
            QAction* p_actn = m_menuProperties->addAction(_form.Name().c_str());

            IRastrColumnsPtr columns{ table->Columns() };
            size_t nrows = IRastrPayload{table->Size()}.Value();
            size_t ncols = IRastrPayload{columns->Count()}.Value();
            if (nrows == 1)
                _form.SetVertical(true);

            for (size_t i = 0 ; i < ncols; i++ ) {
                IRastrColumnPtr column{columns->Item(i)};
                CUIFormField* _field;
                _form.Fields().emplace_back(IRastrPayload{column->Name()}.Value());
            }
            connect(p_actn, &QAction::triggered, [this, _form] {
                openForm(_form); });
        }
    }
}

void MainWindow::setQAstra(const std::shared_ptr<QAstra>& sp_qastra){
    assert(nullptr!=sp_qastra);
    m_sp_qastra = sp_qastra;
    m_RTDM.setQAstra(sp_qastra.get());

    connect(m_sp_qastra.get(), &QAstra::onRastrLog, m_pFormProtocol, &FormProtocol::onRastrLog);
    connect(m_sp_qastra.get(), &QAstra::onRastrLog, m_pMcrWnd, &McrWnd::onRastrLog);

    m_pFormProtocol->setIgnoreAppendProtocol(true);
    assert(nullptr == m_up_PyHlp);
    m_up_PyHlp = std::move( std::make_unique<PyHlp>( *m_sp_qastra->getRastr().get() ) );

    // Менеджер файлов
    m_fileManager = std::make_unique<FileManager>(sp_qastra, this);
}

void MainWindow::setQTI(const std::shared_ptr<QTI>& sp_qti){
    assert(nullptr!=sp_qti);
    m_sp_qti = sp_qti;
}

void MainWindow::setQBarsMDP(const std::shared_ptr<QBarsMDP>& sp_qbarsmdp){
    assert(nullptr!=sp_qbarsmdp);
    m_sp_qbarsmdp = sp_qbarsmdp;
}

void MainWindow::closeEvent(QCloseEvent *event){

    QMainWindow::closeEvent(event);
    writeSettings();
    spdlog::warn( "MainWindow::closeEvent");
    if (m_DockManager) {
        m_DockManager->deleteLater(); //else untabbed window not close!
    }

}

void MainWindow::slot_openGraph(){

#if(!defined(SDL_NO))
    SDL_Init(SDL_INIT_VIDEO); // Basics of SDL, init what you need to use

    auto dw = new ads::CDockWidget( "Графика", this);
    SDLChild * SdlChild = new SDLChild(dw);	// Creating the SDL Window and initializing it.

    dw->setWidget(SdlChild);
    connect( dw, SIGNAL( closed() ), SdlChild, SLOT( OnClose() ) );
    auto area = m_DockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

    SdlChild->SDLInit();
#endif
}

void MainWindow::slot_rgmWrap(){
    emit sig_calcBegin();
    eASTCode code = m_sp_qastra->Rgm("");
    std::string str_msg = "";
    if (code == eASTCode::AST_OK){
        str_msg = "Расчет режима выполнен успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет режима завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_kddWrap(){
    eASTCode code = m_sp_qastra->Kdd("");
    std::string str_msg = "";
    if (code == eASTCode::AST_OK){
        str_msg = "Контроль исходных данных выполнен успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Контроль исходных данных завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
    statusBar()->showMessage( str_msg.c_str(), 0 );
}

void MainWindow::slot_ocWrap(){
    emit sig_calcBegin();
    eASTCode code = m_sp_qastra->Opf("s");

    std::string str_msg = "";
    if (code == eASTCode::AST_OK){
        str_msg = "Оценка состояния выполнена успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет оценки состояния завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_smzuTstWrap(){
    emit sig_calcBegin();

    const std::time_t time_now{ std::time(0) };
    const std::tm*    ptm_now{ std::localtime(&time_now) };
    const std::string str_parameters{ std::to_string(ptm_now->tm_mday) };
    //eASTCode code = m_sp_qastra->SMZU(str_parameters);
    eASTCode code = m_sp_qastra->SMZU("33");

    std::string str_msg = "";
    if (code == eASTCode::AST_OK){
        str_msg = "Расчет МДП выполнен успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет МДП завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
    //onRastrLog(const _log_data& log_data){
    _log_data log_data;
    log_data.lmt = LogMessageTypes::Error;
    log_data.str_msg = str_msg;
    log_data.n_indx = -1;
    log_data.n_stage_id = -1;
    m_sp_qastra->onRastrLog(log_data);
    //m_sp_qastra->onRastrPrint(str_msg);
    //m_sp_qastra->onRastrPrint(str_msg);
    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_tkzWrap(){
    emit sig_calcBegin();
    std::string str_msg = "Implement me in IplainRastr";
    const std::string parameters = "";
    eNonsym Nonsym = eNonsym::KZ_1;
    long    p1 = 1;
    long    p2 =0;
    long    p3 = 0;
    double  LengthFromP1InProc = 0;
    double  rd = 0;
    double  z_re = 0;
    double  z_im = 0;
    const eASTCode code{ m_sp_qastra->Kz( parameters, Nonsym, p1, p2, p3, LengthFromP1InProc, rd = 0, z_re, z_im ) };
    if (code == eASTCode::AST_OK){
        str_msg = "Расчет ТКЗ выполнен успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет ТКЗ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_idopWrap(){
    emit sig_calcBegin();

    eASTCode code = eASTCode::AST_OK;
    formcalcidop* pformcalcidop = new formcalcidop(m_sp_qastra.get(),this);
    pformcalcidop->show();  // on OK call -> calcIdop
    emit sig_calcEnd();
}

void MainWindow::slot_tiRecalcdorWrap()
{
    if (m_sp_qti == nullptr)
    {
        spdlog::warn("{}", "Plugin TI not initialized! Function is unavailable.");
        return;
    }
    emit sig_calcBegin();
    Timer t_calc_recalcdor;

    long code = m_sp_qti->RecalcDor();
    std::string str_msg = "";

    if (code == 1){
        str_msg = fmt::format("Пересчет дорасчетных измерений выполнен за {} сек.",t_calc_recalcdor.seconds());
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Пересчет дорасчетных измерений завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
}

void MainWindow::slot_tiUpdateTablesWrap()
{
    if (m_sp_qti == nullptr)
    {
        spdlog::warn("{}", "Plugin TI not initialized! Function is unavailable.");
        return;
    }
    emit sig_calcBegin();
    Timer _timer;

    long code = m_sp_qti->UpdateTables();
    std::string str_msg = "";

    if (code == 1){
        str_msg = fmt::format("Обновление данных по ТМ (Привязка->T) выполнено за {} сек.",_timer.seconds());
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Пересчет дорасчетных измерений завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }
}

void MainWindow::slot_tiCalcptiWrap(){
    if (m_sp_qti == nullptr)
    {
        spdlog::warn("{}", "Plugin TI not initialized! Function is unavailable.");
        return;
    }

    emit sig_calcBegin();
    Timer t_calc_pti;

    long code = m_sp_qti->CalcPTI();
    std::string str_msg = "";

    if (code == 1){
        str_msg = fmt::format("Расчет ПТИ выполнен за {} сек.",t_calc_pti.seconds());
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет ПТИ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }

    Timer t_add_pti;
    code = m_sp_qti->DobavPTI();
    if (code == 1){
        str_msg = fmt::format("ПТИ записаны в ТИ:Каналы за {} сек. ",t_add_pti.seconds());
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Ошибка записи ПТИ в ТИ:Каналы!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }

    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_tiFiltrtiWrap()
{
    if (m_sp_qti == nullptr)
    {
        spdlog::warn("{}", "Plugin TI not initialized! Function is unavailable.");
        return;
    }

    emit sig_calcBegin();
    Timer t_filtr_ti;

    long code = m_sp_qti->FiltrTI();
    std::string str_msg = "";
    if (code == 1){
        str_msg = fmt::format("Расчет Фильтра ТИ выполнен за {} сек.",t_filtr_ti.seconds());
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет Фильтра ТИ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
    }

    statusBar()->showMessage( str_msg.c_str(), 0 );
    emit sig_calcEnd();
}

void MainWindow::slot_barsMdpPrepareWrap()
{
    if (m_sp_qbarsmdp == nullptr)
    {
        spdlog::warn("{}", "Plugin TI not initialized! Function is unavailable.");
        return;
    }
    emit sig_calcBegin();
    Timer t_barsmdp;

    bool ok{};
    std::string str_msg = "";
    QString text = QInputDialog::getText(this, tr("Подготовка для расчета МДП"),
                                         tr("Сечения:"), QLineEdit::Normal,
                                         "0", &ok);
    if (ok && !text.isEmpty())
    {
        try
        {

            m_sp_qbarsmdp->Init(text.toStdString().c_str());
            m_sp_qbarsmdp->UpdateMDPFields();
            m_sp_qbarsmdp->UpdateAUTOFields();
            str_msg = fmt::format("Подготовка для расчета МДП для сечений {} за {} сек.",text.toStdString().c_str(),t_barsmdp.seconds());
            spdlog::info("{}", str_msg);
        }
        catch(...)
        {
            str_msg = fmt::format("Ошибка в ходе подготовки для расчета МДП для сечений {}!",text.toStdString().c_str());
            spdlog::error("{}", str_msg);
        }
    }
}

void MainWindow::slot_openMcrDialog(){
    McrWnd* pMcrWnd = new McrWnd( this, McrWnd::_en_role::macro_dlg );
    connect(m_sp_qastra.get(), &QAstra::onRastrPrint, pMcrWnd, &McrWnd::onRastrPrint);

    pMcrWnd->setPyHlp(m_up_PyHlp.get());
    pMcrWnd->show();
}

void MainWindow::slot_about(){
    QMessageBox::about( this, tr("About QRastr"), tr("About the <b>QRastr</b>.") );
}

void MainWindow::slot_openForm( QAction* p_actn ){
    const int n_indx = p_actn->data().toInt();
    const auto& forms = m_lstUIForms;

    auto it = forms.begin();
    std::advance(it,n_indx);
    auto form  =*it;
    form.SetName(stringutils::MkToUtf8(form.Name()));
    openForm(form);
}

void MainWindow::openForm(CUIForm _uiform){
    CUIForm form  = _uiform;
    //Проверка существования таблицы
    IRastrTablesPtr tablesx{  m_sp_qastra->getRastr()->Tables() };
    IRastrPayload res{ tablesx->FindIndex(_uiform.TableName()) };
    int t_ind = res.Value();
    if (t_ind < 0)
    {
        spdlog::info( "Таблица [{}] - [{}] не существует! ", form.Name(), form.TableName());
        return;
    }

    spdlog::info( "Прочитана таблица [{}] - [{}]", form.Name(),form.TableName() );
    RtabWidget *prtw = new RtabWidget(m_sp_qastra.get(),form,&m_RTDM,m_DockManager,this);

    //Выравнивание даных по шаблону , выравнивание текста по левому краю
    prtw->widebyshabl();

    QObject::connect(this, &MainWindow::sig_calcBegin, prtw, &RtabWidget::on_calc_begin);
    QObject::connect(this, &MainWindow::sig_calcEnd, prtw, &RtabWidget::on_calc_end);

    // Docking
    //QTitanGrid
    auto dw = new ads::CDockWidget(form.Name().c_str(), this);
    dw->setWidget(prtw->m_grid);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    auto area = m_DockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
    connect(dw, &ads::CDockWidget::closed, prtw, &RtabWidget::OnClose);
}

void MainWindow::slot_itemPressed(const QModelIndex &index){
    int row = index.row();
    int column = index.column();
}

void MainWindow::slot_dataChanged(std::string _t_name, QModelIndex index, QVariant value ){
    std::string tname = _t_name;
}

void MainWindow::slot_dataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value){
}

void MainWindow::slot_rowInserted(std::string _t_name, int _row){
    emit sig_rowInserted(_t_name,_row);
    emit sig_update(_t_name);
}

void MainWindow::slot_rowDeleted(std::string _t_name, int _row){
    emit sig_rowDeleted(_t_name,_row);
    emit sig_update(_t_name);
}

void MainWindow::slot_updateMenu(){

}

void MainWindow::slot_setActiveSubWindow(QWidget *window){
    m_workspace->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::slot_showFormSettings(){
    FormSettings* pformSettings = new FormSettings();
    pformSettings->init(m_sp_qastra);
    pformSettings->show();
}

void MainWindow::createActions(){
    // ========== FileManager ==========

    // Действия открытия/сохранения файлов
    QAction* actNewFile = new QAction(QIcon(":/images/document_new.png"), tr("&Новый"), this);
    actNewFile->setShortcut(tr("Ctrl+N"));
    actNewFile->setStatusTip(tr("Create a new file"));

    connect(actNewFile, &QAction::triggered,
            m_fileManager.get(), &FileManager::newFile);

    QAction* actOpenfile = new QAction(QIcon(":/images/folder_out.png"), tr("&Загрузить"), this);
    actOpenfile->setShortcut(tr("Ctrl+O"));
    actOpenfile->setStatusTip(tr("Open an existing file"));

    connect(actOpenfile, &QAction::triggered,
            m_fileManager.get(), &FileManager::openFiles);

    QAction* actSaveFile = new QAction(QIcon(":/images/disk_blue.png"), tr("&Сохранить"), this);
    actSaveFile->setShortcut(tr("Ctrl+S"));
    actSaveFile->setStatusTip(tr("Save the document to disk"));

    connect(actSaveFile, &QAction::triggered,
            m_fileManager.get(), &FileManager::save);

    QAction* actSaveFileAs = new QAction(tr("&Сохранить как"), this);
    actSaveFileAs->setStatusTip(tr("Save the document under a new name"));

    connect(actSaveFileAs, &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAs);

    QAction* actSaveAllFiles = new QAction(QIcon(":/images/Save_all.png"),tr("&Сохранить все"), this);
    actSaveAllFiles->setStatusTip(tr("Save all the document"));

    connect(actSaveAllFiles, &QAction::triggered,
            m_fileManager.get(), &FileManager::saveAll);

    // Обработка событий файлов
    connect(m_fileManager.get(), &FileManager::fileOpened,
            this, [this](const QString& file) {
                setWindowTitle(file);
            });

    connect(m_fileManager.get(), &FileManager::fileSaved,
            this, [](const QString& file) {
                spdlog::info("File saved: {}", file.toStdString());
            });

    connect(m_fileManager.get(), &FileManager::fileLoadError,
            this, [](const QString& error) {
                spdlog::error("File load error: {}", error.toStdString());
            });

    QAction* actShowFormSettings = new QAction(tr("&Параметры"), this);
    actShowFormSettings->setStatusTip(tr("Open settings form."));
    connect(actShowFormSettings, &QAction::triggered, this, &MainWindow::slot_showFormSettings);
    QAction* actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit the application"));
    connect(actExit, &QAction::triggered, qApp, &QApplication::closeAllWindows);

    //calc
    QAction* actKDD = new QAction(tr("&Контроль"), this);
    actKDD->setStatusTip(tr("Контроль исходных данных"));
    connect(actKDD, &QAction::triggered, this, &MainWindow::slot_kddWrap);
    QAction* actRGM = new QAction(QIcon(":/images/Rastr3_rgm_16x16.png"),tr("&Режим"), this);
    actRGM->setShortcut(tr("F5"));
    actRGM->setStatusTip(tr("Расчет УР"));
    connect(actRGM, &QAction::triggered, this, &MainWindow::slot_rgmWrap);
    QAction* actOC = new QAction(QIcon(":/images/Bee.png"),tr("&ОС"), this);
    actOC->setShortcut(tr("F6"));
    actOC->setStatusTip(tr("Оценка состояния"));
    connect(actOC, &QAction::triggered, this, &MainWindow::slot_ocWrap);
    QAction* actMDP = new QAction(QIcon(":/images/mdp_16.png"),tr("&МДП"), this);
    actMDP->setShortcut(tr("F7"));
    actMDP->setStatusTip(tr("Расчет МДП"));
    connect(actMDP, &QAction::triggered, this, &MainWindow::slot_smzuTstWrap);
    QAction* actTkz = new QAction(QIcon(":/images/TKZ_48.png"),tr("&ТКЗ"), this);
    actTkz->setShortcut(tr("F8"));
    actTkz->setStatusTip(tr("Расчет ТКЗ"));
    connect(actTkz, &QAction::triggered, this, &MainWindow::slot_tkzWrap);
    QAction* actIdop = new QAction(tr("&Доп. ток от Т"), this);
    actIdop->setShortcut(tr("F9"));
    actIdop->setStatusTip(tr("Расчет допустимых токов от температуры"));
    connect(actIdop, &QAction::triggered, this, &MainWindow::slot_idopWrap);

    //graph
    QAction* actGraph = new QAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon)), tr("&graph"), this );
    actGraph->setShortcut(tr("F10"));
    actGraph->setStatusTip(tr("Графика"));
    connect(actGraph, &QAction::triggered, this, &MainWindow::slot_openGraph);
    //macro
    QAction* actMacro = new QAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)), tr("&macro"), this );
    actMacro->setShortcut(tr("F11"));
    actMacro->setStatusTip(tr("Run macro"));
    connect(actMacro, &QAction::triggered, this, &MainWindow::slot_openMcrDialog);
    // TI
    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    QAction* actRecalcDor = new QAction(QIcon(":/images/RecalcDor.png"),tr("&Пересчет дорасчетной ТМ"), this);
    actRecalcDor->setStatusTip(tr("Пересчет дорасчетной ТМ"));
    connect(actRecalcDor, &QAction::triggered, this, &MainWindow::slot_tiRecalcdorWrap);

    QAction* actUpdateTables = new QAction(QIcon(":/images/UpdateTablesTI.png"),tr("&Обновить таблицы по ТМ"), this);
    actUpdateTables->setStatusTip(tr("Обновить таблицы по ТМ"));
    connect(actUpdateTables, &QAction::triggered, this, &MainWindow::slot_tiUpdateTablesWrap);

    QAction* actPTI = new QAction(QIcon(":/images/calc_PTI.png"),tr("&ПТИ"), this);
    actPTI->setStatusTip(tr("Расчет ПТИ"));
    connect(actPTI, &QAction::triggered, this, &MainWindow::slot_tiCalcptiWrap);

    QAction* actFiltrTI = new QAction(QIcon(":/images/filtr_1.png"),tr("&Фильтр ТИ"), this);
    actFiltrTI->setStatusTip(tr("Расчет Фильтр ТИ"));
    connect(actFiltrTI, &QAction::triggered, this, &MainWindow::slot_tiFiltrtiWrap);

    //BarsMDP
    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    QAction* actPrepare_MDP = new QAction(tr("&Подг. МДП"), this);
    actPrepare_MDP->setStatusTip(tr("Подг. МДП"));
    connect(actPrepare_MDP, &QAction::triggered, this, &MainWindow::slot_barsMdpPrepareWrap);

    //windows
    QAction* closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, &QAction::triggered, m_workspace, &QMdiArea::closeActiveSubWindow);

    QAction* closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, &QAction::triggered, m_workspace, &QMdiArea::closeAllSubWindows);

    QAction* tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, &QAction::triggered, m_workspace, &QMdiArea::tileSubWindows);

    QAction* cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, &QAction::triggered, m_workspace, &QMdiArea::cascadeSubWindows);

    QAction* nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcut(tr("Ctrl+F6"));
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, &QAction::triggered, m_workspace, &QMdiArea::activateNextSubWindow);

    QAction* previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcut(tr("Ctrl+Shift+F6"));
    previousAct->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAct, &QAction::triggered, m_workspace, &QMdiArea::activatePreviousSubWindow);

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    //help
    QAction* aboutAct = new QAction(tr("&О программе"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::slot_about);

    //MENU's
    QMenu* menuFile = menuBar()->addMenu(tr("&Файлы"));
    menuFile->addAction(actNewFile);
    menuFile->addAction(actOpenfile);
    menuFile->addAction(actSaveFile);
    menuFile->addAction(actSaveFileAs);
    menuFile->addAction(actSaveAllFiles);
    m_menuProgrammProperties = menuFile->addMenu(tr("&Настройки программы"));
    m_menuProgrammProperties->addAction(actShowFormSettings);
    m_menuProperties = m_menuProgrammProperties->addMenu(tr("&Настройки"));

    menuFile->addSeparator();
    separatorAct = menuFile->addSeparator();
    QMenu* menuFileLast = menuFile->addMenu(tr("&Последние"));
    menuFile->addSeparator();

    menuFile->addSeparator();
    menuFile->addAction(actExit);

    menuBar()->addSeparator();

    QMenu* menuMacro = menuBar()->addMenu(tr("&Макро"));
    menuMacro->addAction(actMacro);
    QMenu* menuGraph = menuBar()->addMenu(tr("&Графика"));
    menuGraph->addAction(actGraph);
    QMenu* menuCalc = menuBar()->addMenu(tr("&Расчеты"));
    menuCalc->addAction(actKDD);
    menuCalc->addAction(actRGM);
    menuCalc->addAction(actOC);
    menuCalc->addAction(actMDP);
    menuCalc->addAction(actPrepare_MDP);
    menuCalc->addAction(actTkz);
    menuCalc->addAction(actIdop);

    m_menuCalcParameters =  menuCalc->addMenu(tr("&Параметры"));
    m_menuCalcTI =  menuCalc->addMenu(tr("&ТИ"));
    m_menuCalcTI->addAction(actPTI);
    m_menuCalcTI->addAction(actRecalcDor);
    m_menuCalcTI->addAction(actUpdateTables);

    m_menuOpen = menuBar()->addMenu(tr("&Открыть") );
    menuBar()->addSeparator();
    QMenu* menuWindow = menuBar()->addMenu(tr("Окна"));
    menuWindow->clear();
    menuWindow->addAction(closeAct);
    menuWindow->addAction(closeAllAct);
    menuWindow->addSeparator();
    menuWindow->addAction(tileAct);
    menuWindow->addAction(cascadeAct);
    menuWindow->addSeparator();
    menuWindow->addAction(nextAct);
    menuWindow->addAction(previousAct);
    menuWindow->addAction(separatorAct);
    QList<QMdiSubWindow *> windows = m_workspace->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());
    QMenu* menuHelp = menuBar()->addMenu(tr("&Помощь"));
    menuHelp->addAction(aboutAct);

    //ToolBars
    QToolBar* toolbarFile = addToolBar(tr("Файл"));
    toolbarFile->addAction(actNewFile);
    toolbarFile->addAction(actOpenfile);
    toolbarFile->addAction(actSaveFile);

    m_toolbarCalc = addToolBar(tr("Расчеты"));
    m_toolbarCalc->addAction(actRGM);
    m_toolbarCalc->addAction(actMDP);
    m_toolbarCalc->addAction(actPrepare_MDP);
    m_toolbarCalc->addAction(actTkz);
    m_toolbarCalc->addAction(actMacro);

    m_toolbarTI = addToolBar(tr("Телеизмерения"));
    m_toolbarTI->addAction(actPTI);
    m_toolbarTI->addAction(actFiltrTI);
    m_toolbarTI->addAction(actOC);
    m_toolbarTI->addAction(actRecalcDor);
    m_toolbarTI->addAction(actUpdateTables);
}

QString MainWindow::strippedName(const QString &fullFileName){
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::createStatusBar(){
    statusBar()->showMessage(tr("Ready"));
}

bool MainWindow::maybeSave(){
    // if (!textEdit->document()->isModified())
    //     return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        //return save();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}
