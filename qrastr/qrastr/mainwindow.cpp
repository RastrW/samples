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

#include "common_qrastr.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>
#include <iostream>
#include <DockManager.h>
#include "mainwindow.h"
#include "rtabwidget.h"
#include "mdiChildTable.h"
#include "mdiChildGrid.h"
#include "mdiChildHeaderGrid.h"
#include "astra_exp.h"
#include "qmcr/mcrwnd.h"
#include "plugin_interfaces.h"
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"
#include "qastra.h"
#include "tsthints.h"
#include "comboboxdelegate.h"
#include "formsettings.h"

MainWindow::MainWindow(){
    m_workspace = new QMdiArea;
    setCentralWidget(m_workspace);
    connect(m_workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateMenus()));
    m_windowMapper = new QSignalMapper(this);
    createActions();
    createStatusBar();
    updateMenus();
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
    if(false){
        QDockWidget *dock = new QDockWidget( "protocol", this);
        dock->setWidget(m_pMcrWnd);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }else{
        static int i = 0;
        auto dw = new ads::CDockWidget( "protocol", this);
        dw->setWidget(m_pMcrWnd);
        int f = ads::CDockWidget::CustomCloseHandling;
        dw->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true);
        //auto area = m_DockManager->addDockWidgetTab(ads::NoDockWidgetArea, dw);
        auto container = m_DockManager->addDockWidgetFloating(dw);
        container->move(QPoint(2100, 20));
        container->resize(1200,800);
    }
    auto qt_sink = std::make_shared<spdlog::sinks::qt_sink_mt>(m_pMcrWnd, "onQStringAppendProtocol");
    auto logg = spdlog::default_logger();
    logg->sinks().push_back(qt_sink);
    //m_pMcrWnd->show();

    setAcceptDrops(true);

}
MainWindow::~MainWindow(){
}
int MainWindow::readSettings(){ //it cache log messages to vector, because it called befor logger intialization
    try{
        QSettings settings(m_pchSettingsOrg);
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 800)).toSize();
        //move(pos);
        //resize(size);
    }catch(const std::exception& ex){
        m_v_cache_log.add( spdlog::level::err,"Exception: {} ", ex.what());
        return -4;
    }catch(...){
        m_v_cache_log.add( spdlog::level::err,"Unknown exception.");
        return -5;
    }
    return 1;
}
int MainWindow::writeSettings(){
    QSettings settings(m_pchSettingsOrg);
    //QSettings::IniFormat
    QString qstr = settings.fileName();
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    return 1;
}
void MainWindow::tst_onRastrHint(const _hint_data& dh){

    spdlog::info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    spdlog::info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    spdlog::info("XXX MainWindow::tst_onRastrHint about {} {} {} {} {} XXX"
        , QAstra::getHintName(dh.hint)
        , static_cast<std::underlying_type<EventHints>::type>(dh.hint)
        , dh.str_table
        , dh.str_column
        , dh.n_indx
    );
    spdlog::info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    spdlog::info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
}
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
   event->acceptProposedAction();
}
void MainWindow::dropEvent(QDropEvent *dropEvent)
{
   QStringList filePathList;
   foreach (QUrl url, dropEvent->mimeData()->urls())
   {
       std::string fileName = url.toLocalFile().toStdString();
       filePathList << url.toLocalFile();
       m_sp_qastra->LoadFile(eLoadCode::RG_REPL, fileName,"");
   }
   dropEvent->acceptProposedAction();
}
void MainWindow::logCacheFlush(){
    for( const auto& cache_log : m_v_cache_log){
        spdlog::log(cache_log.lev, cache_log.str_log);
    }
    m_v_cache_log.clear();
}
void MainWindow::showEvent( QShowEvent* event ){
        QWidget::showEvent( event );
        //your code here
        // https://stackoverflow.com/questions/14161100/which-qt-widget-should-i-use-for-message-display
}
MainWindow::_cache_log::_cache_log( const spdlog::level::level_enum lev_in, std::string_view sv_in )
    : lev{lev_in}
    , str_log{sv_in}{
}
MainWindow::_cache_log& MainWindow::_cache_log::operator=(const MainWindow::_cache_log& cache_log){
    lev     = cache_log.lev;
    str_log = cache_log.str_log;
    return *this;
}
MainWindow::_cache_log& MainWindow::_cache_log::operator=(const MainWindow::_cache_log&& cache_log){
    operator=(cache_log);
    return *this;
}
MainWindow::_cache_log::_cache_log(const MainWindow::_cache_log& cache_log){
    operator=(cache_log);
}
MainWindow::_cache_log::_cache_log(const MainWindow::_cache_log&& cache_log){
    operator=(cache_log);
}
template <typename... Args>
void MainWindow::_v_cache_log::add( const spdlog::level::level_enum lev_in, const std::string_view sv_format, Args&&... args ){
    _cache_log cache_log{lev_in, fmt::format(sv_format, args...)};
    emplace_back(cache_log);
}
void MainWindow::setForms(const std::list<CUIForm>& forms){ // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    int i = 0;
    m_lstUIForms = forms;
    QMap<QString,QMenu *> map_menu;
    for(const auto& j_form : forms){
        std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
        if (!str_MenuPath.empty() && str_MenuPath.at(0) == '_')
            continue;
        QString qstr_MenuPath = str_MenuPath.c_str();
        if (!str_MenuPath.empty() && !map_menu.contains(qstr_MenuPath))
            map_menu.insert(qstr_MenuPath,m_menuOpen->addMenu(str_MenuPath.c_str()));
    }
    for(const auto& j_form : forms){
        std::string str_Name = stringutils::cp1251ToUtf8(j_form.Name());
        std::string str_TableName = j_form.TableName();
        std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
        QString qstr_MenuPath = str_MenuPath.c_str();
        QMenu* cur_menu = m_menuOpen;
        if (map_menu.contains(qstr_MenuPath))
            cur_menu = map_menu[qstr_MenuPath];
        if (!str_Name.empty() && str_Name.at(0) != '_'){
            QAction* p_actn = cur_menu->addAction(str_Name.c_str());
            p_actn->setData(i);
        }
        i++;
    }
    connect( m_menuOpen, SIGNAL(triggered(QAction *)), this, SLOT(onOpenForm(QAction *)), Qt::UniqueConnection);
}
void MainWindow::setQAstra(const std::shared_ptr<QAstra>& sp_qastra){
    assert(nullptr!=sp_qastra);
    m_sp_qastra = sp_qastra;

    connect( m_sp_qastra.get(), SIGNAL(onRastrLog(const _log_data&) ), m_pMcrWnd, SLOT(onRastrLog(const _log_data&)));

    if(true){
        //vetv
        TstHints* tstHints_vetv = new TstHints(this);
        tstHints_vetv->setQAstra(std::weak_ptr<QAstra>(m_sp_qastra));
        tstHints_vetv->setTableName("vetv");
        tstHints_vetv->setColNames({"ip","iq","np","name","pl_ip","slb"});
        const bool my0 = QObject::connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), tstHints_vetv, SLOT( onRastrHint(const _hint_data&) ) );
        assert(my0 == true);
        auto dw_tst_hints_vetv = new ads::CDockWidget( "TstHints", this);
        dw_tst_hints_vetv->setWidget(tstHints_vetv);
        dw_tst_hints_vetv->move(QPoint(20, 20));
        auto container_tsthints_vetv = m_DockManager->addDockWidgetFloating(dw_tst_hints_vetv);
        container_tsthints_vetv->move(QPoint(1800,10));
        container_tsthints_vetv->resize(600,400);
        //node
        TstHints* tstHints = new TstHints(this);
        tstHints->setQAstra(std::weak_ptr<QAstra>(m_sp_qastra));
        tstHints->setTableName("node");
        tstHints->setColNames({"ny","name","vras","delta"});
        const bool my1 = QObject::connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), tstHints, SLOT( onRastrHint(const _hint_data&) ) );
        assert(my1 == true);
        auto dw_tst_hints = new ads::CDockWidget( "TstHints", this);
        dw_tst_hints->setWidget(tstHints);
        dw_tst_hints->move(QPoint(20, 20));
        auto container_tsthints = m_DockManager->addDockWidgetFloating(dw_tst_hints);
        container_tsthints->move(QPoint(1400,20));
        container_tsthints->resize(600,400);
/*
        static int colCount = 50;
        static int rowCount = 300;
        tstHints->setColumnCount(colCount);
        tstHints->setRowCount(rowCount);
        for (int col = 0; col < colCount; ++col){
          tstHints->setHorizontalHeaderItem(col, new QTableWidgetItem(QString("Col %1").arg(col+1)));
          for (int row = 0; row < rowCount; ++row){
             tstHints->setItem(row, col, new QTableWidgetItem(QString("T %1-%2").arg(row + 1).arg(col+1)));
          }
        }
*/
    }
}
void MainWindow::closeEvent(QCloseEvent *event){
    QMainWindow::closeEvent(event);
    writeSettings();
    spdlog::warn( "MainWindow::closeEvent");
    if (m_DockManager) {
        m_DockManager->deleteLater(); //else untabbed window not close!
    }
#if(!defined(QICSGRID_NO))
    m_workspace->closeAllSubWindows();
    if (activeMdiChild()) {
        event->ignore();
    } else {
        event->accept();
    }
#endif// #if(!defined(QICSGRID_NO))
}
void MainWindow::newFile(){
#if(!defined(QICSGRID_NO))
    MdiChild *child = createMdiChild(  j_forms_[0] );
    child->newFile();
    child->show();
#endif
}
void MainWindow::open(){
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
#if(!defined(QICSGRID_NO))
        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            m_workspace->setActiveSubWindow(existing);
            return;
        }
#endif//#if(!defined(QICSGRID_NO))
        m_sp_qastra->LoadFile(eLoadCode::RG_REPL, fileName.toStdString(),"");
    }
}
void MainWindow::save(){
#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif//#if(!defined(QICSGRID_NO))
    if (!m_cur_file.empty()){
        int nRes = 0;
        const std::string& f = m_cur_file;
        assert(!"not implemented");
        if(nRes>0){
            std::string str_msg = fmt::format( "{}: {}", "File saved", f);
            statusBar()->showMessage( str_msg.c_str(), 2000 );
        } else {
            std::string str_msg = fmt::format( "{}: {}", "File not saved", f);
            QMessageBox msgBox;
            msgBox.critical( this, tr("File not saved"), str_msg.c_str() );
        }
    }
}
void MainWindow::saveAs(){
#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif
    QString fileName = QFileDialog::getSaveFileName(this);
    if (!fileName.isEmpty()){
        int nRes = 0;
        std::string f = fileName.toUtf8().constData(); //  it works!!!
        assert(!"not implemented");
        if(nRes>0){
            std::string str_msg = fmt::format( "{}: {}", "File saved", f);
            statusBar()->showMessage( str_msg.c_str(), 2000 );
        } else {
            std::string str_msg = fmt::format( "{}: {}", "File not saved", f);
            QMessageBox msgBox;
            msgBox.critical( this, tr("File not saved"), str_msg.c_str() );
        }
    }
}
void MainWindow::rgm_wrap(){
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
    emit rgm_signal();
}
void MainWindow::onDlgMcr(){
    McrWnd* pMcrWnd = new McrWnd(this) ;
    pMcrWnd->show();
}
void MainWindow::about(){
   QMessageBox::about( this, tr("About QRastr"), tr("About the <b>QRastr</b>.") );
}
void MainWindow::onOpenForm( QAction* p_actn ){
    const int n_indx = p_actn->data().toInt();
    const auto& forms = m_lstUIForms;

    auto it = forms.begin();
    std::advance(it,n_indx);
    auto form  =*it;
    qDebug() << "\n Open form:" + form.Name();
    spdlog::info( "Create tab [{}]", stringutils::cp1251ToUtf8(form.Name()) );
    RtabWidget *prtw = new RtabWidget(m_sp_qastra.get(),form,this);

    /*
    connect(this, &MainWindow::file_loaded,  prtw, &RtabWidget::onFileLoad);    //Загрузка файла
    connect(this, &MainWindow::rgm_signal, prtw, &RtabWidget::update_data);     //Расчет УР
    //RModel вызывает изменение Data: Запомнить изменение Data в MainWindow и из MainWindow вызывть изменение RModel во всех сущьностях
    connect(prtw->prm, SIGNAL(dataChanged(std::string,std::string,int,QVariant)),
                 this, SLOT(ondataChanged(std::string,std::string,int,QVariant)));
    connect(this,      SIGNAL(rm_change(std::string,std::string,int,QVariant)),
       prtw->prm,      SLOT(onRModelchange(std::string,std::string,int,QVariant)));
    //MainWindow: вызывть изменение RModel во всех сущьностях
    connect(prtw->prm, SIGNAL(RowInserted(std::string,int)),
                  this,SLOT(onRowInserted(std::string,int)));
    connect(this,      SIGNAL(rm_RowInserted(std::string,int)),
       prtw->prm,      SLOT(onrm_RowInserted(std::string,int)));
    //Удаление строки
    connect(prtw->prm, SIGNAL(RowDeleted(std::string,int)),
                  this,SLOT(onRowInserted(std::string,int)));
    connect(this,      SIGNAL(rm_RowDeleted(std::string,int)),
       prtw->prm,      SLOT(onrm_RowDeleted(std::string,int)));
    connect(this,      SIGNAL(rm_update(std::string)),
            prtw,      SLOT(onUpdate(std::string)));
*/
    // Docking
    if(false){
        QDockWidget *dock = new QDockWidget( stringutils::cp1251ToUtf8(form.Name()).c_str(), this);
        dock->setWidget(prtw);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }else{
        static int i = 0;
        auto dw = new ads::CDockWidget( stringutils::cp1251ToUtf8(form.Name()).c_str(), this);
        dw->setWidget(prtw);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        auto area = m_DockManager->addDockWidgetTab(ads::CenterDockWidgetArea, dw);
        qDebug() << "doc dock widget created!" << dw << area;
    }
    prtw->show();
#if(!defined(QICSGRID_NO))
    const nlohmann::json j_form = up_rastr_->GetJForms()[n_indx];
    MdiChild *child = createMdiChild( j_form );
    //child->newFile();
    child->show();
#endif // #if(!defined(QICSGRID_NO))
}
void MainWindow::onItemPressed(const QModelIndex &index){
    //prtw_current = qobject_cast<RtabWidget*>(index.);
    int row = index.row();
    int column = index.column();
    //QTableView* t = index.parent();
    qDebug()<<"Pressed:" <<row<< ","<<column;
}
void MainWindow::ondataChanged(std::string _t_name, QModelIndex index, QVariant value ){
    std::string tname = _t_name;
    //emit rm_change(_t_name,index,value);
}
void MainWindow::ondataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value){
    //emit rm_change(_t_name,_col_name,_row,_value);
}
void MainWindow::onRowInserted(std::string _t_name, int _row){
    emit rm_RowInserted(_t_name,_row);
    emit rm_update(_t_name);
}
void MainWindow::onRowDeleted(std::string _t_name, int _row){
    emit rm_RowDeleted(_t_name,_row);
    emit rm_update(_t_name);
}
void MainWindow::onButton2Click(){
    const long num_chars = 10000;
    char* pch_JSON_out = new char[num_chars];
    //long n_res = PyRunMacro( L"", L"", pch_JSON_out, num_chars );
}
void MainWindow::updateMenus(){
#if(!defined(QICSGRID_NO))
    bool hasMdiChild = (activeMdiChild() != 0);
    m_saveAct->setEnabled(hasMdiChild);
    m_saveAsAct->setEnabled(hasMdiChild);
    m_pasteAct->setEnabled(hasMdiChild);
    m_closeAct->setEnabled(hasMdiChild);
    m_closeAllAct->setEnabled(hasMdiChild);
    m_tileAct->setEnabled(hasMdiChild);
    m_cascadeAct->setEnabled(hasMdiChild);
    m_nextAct->setEnabled(hasMdiChild);
    m_previousAct->setEnabled(hasMdiChild);
    m_separatorAct->setVisible(hasMdiChild);
#endif //#if(!defined(QICSGRID_NO))
}
void MainWindow::setActiveSubWindow(QWidget *window){
    m_workspace->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}
void MainWindow::showFormSettings(){
    FormSettings* pformSettings = new FormSettings();
    pformSettings->init();
    pformSettings->show();
}
void MainWindow::createActions(){
    //file
    QAction* newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    QAction* openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    QAction* saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    QAction* saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    QAction* actShowFormSettings = new QAction(tr("S&ettings..."), this);
    actShowFormSettings->setStatusTip(tr("Open settings form."));
    connect(actShowFormSettings, SIGNAL(triggered()), this, SLOT(showFormSettings()));
    QAction* exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    //macro
    QAction* ActMacro = new QAction(QIcon(":/images/cut.png"),tr("&macro"), this);
    ActMacro->setShortcut(tr("F11"));
    ActMacro->setStatusTip(tr("Run macro"));
    connect(ActMacro, SIGNAL(triggered()), this, SLOT(onDlgMcr()));
    //calc
    QAction* actRGM = new QAction(QIcon(":/images/Rastr3_rgm_16x16.png"),tr("&rgm"), this);
    actRGM->setShortcut(tr("F5"));
    actRGM->setStatusTip(tr("Calc rgm"));
    connect(actRGM, SIGNAL(triggered()), this, SLOT(rgm_wrap()));
    //windows
    QAction* closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()), m_workspace, SLOT(closeActiveSubWindow()));
    QAction* closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()), m_workspace, SLOT(closeAllSubWindows()));
    QAction* tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), m_workspace, SLOT(tileSubWindows()));
    QAction* cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), m_workspace, SLOT(cascadeSubWindows()));
    QAction* nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcut(tr("Ctrl+F6"));
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()), m_workspace, SLOT(activateNextSubWindow()));
    QAction* previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcut(tr("Ctrl+Shift+F6"));
    previousAct->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAct, SIGNAL(triggered()), m_workspace, SLOT(activatePreviousSubWindow()));
    QAction* separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    //help
    QAction* aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    //MENU's
    QMenu* menuFile = menuBar()->addMenu(tr("&File"));
    menuFile->addAction(newAct);
    menuFile->addAction(openAct);
    menuFile->addAction(saveAct);
    menuFile->addAction(actShowFormSettings);
    menuFile->addSeparator();
    menuFile->addAction(exitAct);
    QMenu* menuMacro = menuBar()->addMenu(tr("&Macro"));
    menuMacro->addAction(ActMacro);
    QMenu* menuCalc = menuBar()->addMenu(tr("&Calc"));
    menuCalc->addAction(actRGM);
    m_menuOpen = menuBar()->addMenu(tr("&Open") );
    menuBar()->addSeparator();
    QMenu* menuWindow = menuBar()->addMenu(tr("&Window"));
    //connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));// ustas about: not understend for a what this? (have fall on it)
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
#if(!defined(QICSGRID_NO))
    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        if ( !child ) printf("uh oh!\n");
        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = m_windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), m_windowMapper, SLOT(map()));
        m_windowMapper->setMapping(action, windows.at(i));
    }
#endif
    QMenu* menuHelp = menuBar()->addMenu(tr("&Help"));
    menuHelp->addAction(aboutAct);

    //ToolBars
    QToolBar* toolbarFile = addToolBar(tr("File"));
    toolbarFile->addAction(newAct);
    toolbarFile->addAction(openAct);
    toolbarFile->addAction(saveAct);
    m_toolbarCalc = addToolBar(tr("Calc"));
    m_toolbarCalc->addAction(actRGM);

    //XZ
    createCalcLayout();
}
void MainWindow::Btn1_onClick(){
    //Show stock grid with nodes table
    QTableView* ptv = new QTableView();
    RModel* pmm = new RModel(nullptr,m_sp_qastra.get());
    const auto& forms = m_lstUIForms;
    auto it = forms.begin();
    std::advance(it,0);
    auto form  =*it;
    pmm->setForm(&form);
    pmm->populateDataFromRastr();
    ptv->setSortingEnabled(true);
    ptv->setModel(pmm);
    ptv->show();
}
void MainWindow::Btn3_onClick(){
    //Show test ComboBox item at grid column

    QStandardItemModel* model = new QStandardItemModel(8, 2);
    QTableView* ptableView = new QTableView();;
    ComboBoxDelegate* delegate = new ComboBoxDelegate(this,"Item1,Item2,Item3");
    //tableView.setItemDelegate(&delegate);
    ptableView->setItemDelegateForColumn(1, delegate); // Column 0 can take any value, column 1 can only take values up to 8.

    for (int row = 0; row < 8; ++row)
    {
        for (int column = 0; column < 2; ++column)
        {
            QModelIndex index = model->index(row, column, QModelIndex());
            int value = (row+1) * (column+1);
            std::cout << "Setting (" << row << ", " << column << ") to " << value << std::endl;
            model->setData(index, QVariant(value));
        }
    }
    // Make the combo boxes always displayed.
    for ( int i = 0; i < model->rowCount(); ++i )
    {
        ptableView->openPersistentEditor( model->index(i, 1) );
    }
    ptableView->setModel(model);
    ptableView->show();
}
void MainWindow::createCalcLayout(){
    // набор вложенных виджетов - кнопок
    QPushButton *btn1 = new QPushButton("Stock grid");
    QPushButton *btn2 = new QPushButton("Button 2");
    QPushButton *btn3 = new QPushButton("Tst ComboBoxDelegate");

    connect(btn1,&QPushButton::clicked,this, &MainWindow::Btn1_onClick);
    connect(btn2,&QPushButton::clicked,this, &MainWindow::onButton2Click);
    connect(btn3,&QPushButton::clicked,this, &MainWindow::Btn3_onClick);

    QWidget* widget = new QWidget;
    widget -> setWindowTitle("Functions");
    m_layoutActions = new QHBoxLayout(widget);
    m_layoutActions->addWidget(btn1);
    m_layoutActions->addWidget(btn3);
    m_toolbarCalc->addWidget(widget);
}
void MainWindow::createStatusBar(){
    statusBar()->showMessage(tr("Ready"));
}
#if(!defined(QICSGRID_NO))
MdiChild *MainWindow::createMdiChild( nlohmann::json j_form ){
    //MdiChild* child = new MdiChild(id_rastr_, j_form ,mdiChildGrid::createGrid,mdiChildHeaderGrid::createHeaderGrid,this);
    MdiChild* child = new MdiChild( up_rastr_->GetRastrId(), j_form, mdiChildGrid::createGrid,mdiChildHeaderGrid::createHeaderGrid,this);
    QObject::connect(this, SIGNAL(rgm_signal()), child, SLOT(update_data()));
    //QObject::connect(child, SIGNAL(rowsDeleted()), child, SLOT(update_data()));
    m_workspace->addSubWindow(child);
    return child;
}
MdiChild *MainWindow::activeMdiChild(){
    if (m_workspace->activeSubWindow())
        return qobject_cast<MdiChild *>(m_workspace->activeSubWindow()->widget());
    return 0;
}
QicsTable* MainWindow::activeTable(){
    QMdiSubWindow* activeWindow = m_workspace->activeSubWindow();
    if(!activeWindow)
        return 0;
    QicsTable *table = static_cast<QicsTable*>(activeMdiChild());
    return table;
}
QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName){
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QMdiSubWindow *subWindow, m_workspace->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(subWindow->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return subWindow;
    }
    return 0;
}
void MainWindow::sortAscending(){
    QicsTable *table = activeTable();
    if (table) {
        QicsSelectionList *list = table->selectionList(true);
        if (!list)
            return;
        QVector<int> selectedCols = list->columns().toVector();
        if (selectedCols.size() <= 0) selectedCols << 0;
        //QicsRegion reg = list->region();
        //table->sortRows(selectedCols, Qics::Ascending, reg.startRow(), reg.endRow());
        table->sortRows(selectedCols, Qics::Ascending);
    }
}
void MainWindow::sortDescending(){
    QicsTable *table = activeTable();
    if (table) {
        QicsSelectionList *list = table->selectionList(true);
        if (!list)
            return;
        QVector<int> selectedCols = list->columns().toVector();
        if (selectedCols.size() <= 0) selectedCols << 0;
        //QicsRegion reg = list->region();
        //table->sortRows(selectedCols, Qics::Ascending, reg.startRow(), reg.endRow());
        table->sortRows(selectedCols, Qics::Descending);
    }
}
#endif //#if(!defined(QICSGRID_NO))

