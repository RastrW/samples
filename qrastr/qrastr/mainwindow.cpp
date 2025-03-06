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
//#include "mdiChildTable.h"
//#include "mdiChildGrid.h"
//#include "mdiChildHeaderGrid.h"
#include "astra_exp.h"
#include "qmcr/mcrwnd.h"
#include "plugin_interfaces.h"
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"
#include "qastra.h"
#include "tsthints.h"
#include "delegatecombobox.h"
#include "formsettings.h"
#include "rtabwidget.h"
#include "params.h"
#include "formfilenew.h"
#include "testmodel.h"
#include "formprotocol.h"
#include <QtitanDef.h>
#include <QtitanGrid.h>


MainWindow::MainWindow(){
    m_workspace = new QMdiArea;
    setCentralWidget(m_workspace);
    connect(m_workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateMenus()));
    m_windowMapper = new QSignalMapper(this);
    createActions();
    createStatusBar();
    updateMenus();
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
    if(false){
        QDockWidget *dock = new QDockWidget( "protocol", this);
        dock->setWidget(m_pMcrWnd);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }else{
        auto dw = new ads::CDockWidget( "protocol", this);
        dw->setWidget(m_pMcrWnd);
        int f = ads::CDockWidget::CustomCloseHandling;
        dw->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true);
        //auto area = m_DockManager->addDockWidgetTab(ads::NoDockWidgetArea, dw);
        auto container = m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea,dw);
       /* auto container = m_DockManager->addDockWidgetFloating(dw);
        container->move(QPoint(2100, 20));
        container->resize(1200,800);*/

        auto pdwProtocol = new ads::CDockWidget( "protocolMain", this );
        pdwProtocol->setWidget(m_pFormProtocol);
        //int f = ads::CDockWidget::CustomCloseHandling;
        pdwProtocol->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true );
        //auto area = m_DockManager->addDockWidgetTab(ads::NoDockWidgetArea, dw);
        auto pfdc = m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea,pdwProtocol);
       // auto pfdc = m_DockManager->addDockWidgetFloating(pdwProtocol);
        //pfdc->move(QPoint(2100, 20));
       // pfdc->resize(600,400);


    }
    auto qt_sink = std::make_shared<spdlog::sinks::qt_sink_mt>(m_pMcrWnd, "onQStringAppendProtocol");
    auto logg = spdlog::default_logger();
    logg->sinks().push_back(qt_sink);

    auto qsinkProtocol = std::make_shared<spdlog::sinks::qt_sink_mt>(m_pFormProtocol, "onAppendProtocol");
    //auto logg = spdlog::default_logger();
    logg->sinks().push_back(qsinkProtocol);

    setAcceptDrops(true);


}
MainWindow::~MainWindow()
{
}

int MainWindow::readSettings(){ //it cache log messages to vector, because it called befor logger intialization
    try{

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
    m_RTDM.SetForms(&m_lstUIForms);
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
    m_RTDM.setQAstra(sp_qastra.get());

    connect( m_sp_qastra.get(), SIGNAL(onRastrLog(const _log_data&) ), m_pFormProtocol, SLOT(onRastrLog(const _log_data&)) );
    connect( m_sp_qastra.get(), SIGNAL(onRastrLog(const _log_data&) ), m_pMcrWnd,       SLOT(onRastrLog(const _log_data&)) );
    m_pFormProtocol->setIgnoreAppendProtocol(true);

    if(false){
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

   /* if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }*/

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
    FormFileNew* pformFileNew = new FormFileNew(this);
    if(QDialog::Accepted == pformFileNew->exec()){
        const FormFileNew::_s_checked_templatenames s_checked_templatenames = pformFileNew->getCheckedTemplateNames();
        for(const FormFileNew::_s_checked_templatenames::value_type& templatename : s_checked_templatenames){
            const std::string str_path_to_shablon = Params::GetInstance()->getDirSHABLON().absolutePath().toStdString() + "//" +templatename;
            m_sp_qastra->Load( eLoadCode::RG_REPL, "", str_path_to_shablon );
        }
    }
#if(!defined(QICSGRID_NO))
    MdiChild *child = createMdiChild(  j_forms_[0] );
    child->newFile();
    child->show();
#endif
}
void MainWindow::open(){
    QFileDialog fileDlg( this, tr("Open Rastr files") );
    fileDlg.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDlg.setViewMode(QFileDialog::Detail);
    QString qstr_filter;
    qstr_filter += "Known types(";
    const Params::_v_template_exts v_template_ext{ Params::GetInstance()->getTemplateExts() };
    for(const Params::_v_template_exts::value_type& template_ext : v_template_ext){
        qstr_filter += QString("*%1 ").arg(template_ext.second.c_str());
    }
    qstr_filter += ");;";
    const QString qstr_filter_no_template {"No template (*)"};
    qstr_filter += qstr_filter_no_template; //qstr_filter += QString("xz (*.rg2 *.os)");
    fileDlg.setNameFilter(qstr_filter);
    fileDlg.setFileMode(QFileDialog::ExistingFiles); //ExistingFile
    int n_res = fileDlg.exec();
    if(QDialog::Accepted == n_res){
        const QString selectedFilter = fileDlg.selectedNameFilter();
        for(const auto& rfile : fileDlg.selectedFiles()){
            spdlog::info("try load file: {}", rfile.toStdString());
            //spdlog::info("try load file: %1", rfile.toUtf8().constData());
            if(qstr_filter_no_template != selectedFilter){
                bool bl_find_template = false;
                for(const Params::_v_template_exts::value_type& template_ext : v_template_ext){
                    if(true == rfile.endsWith(template_ext.second.c_str())){
                        bl_find_template = true;
                        const std::string str_path_to_shablon = Params::GetInstance()->getDirSHABLON().absolutePath().toStdString() + "//" +template_ext.first +template_ext.second;
                        m_sp_qastra->Load( eLoadCode::RG_REPL, rfile.toStdString(), str_path_to_shablon );
                        setCurrentFile(rfile, str_path_to_shablon);
                        setWindowTitle(rfile);
                        break;
                    }
                }
                if(false == bl_find_template){
                    spdlog::error("Template not found! for: ", rfile.toStdString());
                }
            }else{
                IPlainRastrRetCode res = m_sp_qastra->Load( eLoadCode::RG_REPL, rfile.toStdString(), "" );
                setWindowTitle(rfile);
                setCurrentFile(rfile);
                if (res ==IPlainRastrRetCode::Ok )
                    spdlog::info("File loaded {}", rfile.toStdString());
                else
                {
                    spdlog::info("Error File load result = {}", static_cast<int>(res));
                }

            }
        }
    }

    return;

    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
#if(!defined(QICSGRID_NO))
        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            m_workspace->setActiveSubWindow(existing);
            return;
        }
#endif//#if(!defined(QICSGRID_NO))
        m_sp_qastra->Load(eLoadCode::RG_REPL, fileName.toStdString(),"");
        setWindowTitle(fileName);
        m_cur_file = fileName.toStdString();
    }
}
void MainWindow::saveAs(){
    QFileDialog fileDlg( this, tr("Save Rastr file") );
    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDlg.setViewMode(QFileDialog::Detail);
    QString qstr_filter;
    const Params::_v_template_exts v_template_ext{ Params::GetInstance()->getTemplateExts() };
    for(const Params::_v_template_exts::value_type& template_ext : v_template_ext){
        qstr_filter += QString("%1 (*%2);;").arg(template_ext.first.c_str()).arg(template_ext.second.c_str());
    }
    const QString qstr_filter_no_template {"No template (*)"};
    qstr_filter += qstr_filter_no_template; //qstr_filter += QString("xz (*.rg2 *.os)");
    fileDlg.setNameFilter(qstr_filter);
    fileDlg.selectNameFilter("режим (*.rg2)");
    fileDlg.setFileMode(QFileDialog::AnyFile);
    int n_res = fileDlg.exec();
    if(QDialog::Accepted == n_res){
        const QString qstr_template = fileDlg.selectedNameFilter();
        const QString qstr_rfile    = fileDlg.selectedFiles()[0];
        const std::string str_path_to_shablon = Params::GetInstance()->getDirSHABLON().absolutePath().toStdString() + "//" +qstr_template.toStdString();
        //m_sp_qastra->LoadFile( eLoadCode::RG_REPL, "", str_path_to_shablon );
        m_sp_qastra->Save( qstr_rfile.toStdString(), str_path_to_shablon );
        setCurrentFile(qstr_rfile);
        qDebug() << "templ: "<< qstr_template << "  file : " << qstr_rfile ;
    }

#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif//#if(!defined(QICSGRID_NO))
    if (!m_cur_file.empty()){
        int nRes = 0;
        const std::string& f = m_cur_file;
        //assert(!"not implemented");


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
void MainWindow::save(){

    m_sp_qastra->Save( m_cur_file, "" );
    std::string str_msg = fmt::format( "{}: {}", "Сохранен файл", m_cur_file);
    return;

    QString fileName = QFileDialog::getSaveFileName(this);
    if (!fileName.isEmpty()){
        int nRes = 0;
        std::string f = fileName.toUtf8().constData(); //  it works!!!
        m_sp_qastra->Save( f, "" );
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
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        //loadFile(action->data().toString());
        QString _fileshabl = action->data().toString();
        QStringList qslist = _fileshabl.split(" ");
        std::string file = qslist[0].toStdString();
        std::string shabl = "";
        if (qslist.size() > 1)
        {
            shabl = qslist[1].toStdString();
            shabl.erase(shabl.begin());
            shabl.erase(shabl.end()-1);
        }

        //m_sp_qastra->Load( eLoadCode::RG_REPL, action->data().toString().toStdString(), "" );
        m_sp_qastra->Load( eLoadCode::RG_REPL, file, shabl );
        setWindowTitle(action->data().toString());
    }
}
void MainWindow::rgm_wrap(){
    emit signal_calc_begin();
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
    emit signal_calc_end();
    //emit rgm_signal();
}
void MainWindow::oc_wrap(){
    emit signal_calc_begin();
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
    emit signal_calc_end();
    //emit rgm_signal();
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
    RtabWidget *prtw = new RtabWidget(m_sp_qastra.get(),form,&m_RTDM,m_DockManager,this);

    QObject::connect(this, &MainWindow::signal_calc_begin, prtw, &RtabWidget::on_calc_begin);
    QObject::connect(this, &MainWindow::signal_calc_end, prtw, &RtabWidget::on_calc_end);

    // Docking
    if(false){
        QDockWidget *dock = new QDockWidget( stringutils::cp1251ToUtf8(form.Name()).c_str(), this);
        dock->setWidget(prtw);
        //dock->setWidget(prtw->m_grid);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }else{
        //QTitanGrid
        auto dw = new ads::CDockWidget( stringutils::cp1251ToUtf8(form.Name()).c_str(), this);
        dw->setWidget(prtw->m_grid);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        auto area = m_DockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        connect( dw, SIGNAL( closed() ),
                prtw, SLOT( OnClose() ) );                    // emit RtabWidget->closeEvent

        // Stock QT grid
        /*auto dw2 = new ads::CDockWidget( stringutils::cp1251ToUtf8(form.Name()).c_str(), this);
        dw2->setWidget(prtw);
        dw2->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        dw2->setFeature(ads::CDockWidget::DockWidgetForceCloseWithArea, true);
        auto area2 = m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea, dw2);
        connect( dw2, SIGNAL( closed() ),
                prtw, SLOT( OnClose() ) );                    // emit RtabWidget->closeEvent
        */

        qDebug() << "doc dock widget created!" << dw << area;
    }
    //prtw->show();
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
    pformSettings->init(m_sp_qastra);
    pformSettings->show();
}
void MainWindow::createActions(){
    //file
    QAction* newAct = new QAction(QIcon(":/images/new.png"), tr("&Новый"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    QAction* openAct = new QAction(QIcon(":/images/open.png"), tr("&Загрузить"), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    QAction* saveAct = new QAction(QIcon(":/images/save.png"), tr("&Сохранить"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    QAction* saveAsAct = new QAction(tr("&Сохранить как"), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    QAction* actShowFormSettings = new QAction(tr("&Настройки программы"), this);
    actShowFormSettings->setStatusTip(tr("Open settings form."));
    connect(actShowFormSettings, SIGNAL(triggered()), this, SLOT(showFormSettings()));
    QAction* exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    //macro
    QAction* ActMacro = new QAction(QIcon(":/images/cut.png"),tr("&macro"), this);
    ActMacro->setShortcut(tr("F11"));
    ActMacro->setStatusTip(tr("Run macro"));
    connect(ActMacro, SIGNAL(triggered()), this, SLOT(onDlgMcr()));
    //calc
    QAction* actRGM = new QAction(QIcon(":/images/Rastr3_rgm_16x16.png"),tr("&Режим"), this);
    actRGM->setShortcut(tr("F5"));
    actRGM->setStatusTip(tr("Calc rgm"));
    connect(actRGM, SIGNAL(triggered()), this, SLOT(rgm_wrap()));
    QAction* actOC = new QAction(QIcon(":/images/Bee.png"),tr("&ОС"), this);
    actOC->setShortcut(tr("F6"));
    actOC->setStatusTip(tr("Оценка состояния"));
    connect(actOC, SIGNAL(triggered()), this, SLOT(oc_wrap()));
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
    //QAction* separatorAct = new QAction(this);
    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    //help
    QAction* aboutAct = new QAction(tr("&О программе"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));


    //MENU's
    //QMenu* menuFile = menuBar()->addMenu(tr("&File"));
    QMenu* menuFile = menuBar()->addMenu(tr("&Файлы"));
    menuFile->addAction(newAct);
    menuFile->addAction(openAct);
    menuFile->addAction(saveAct);
    menuFile->addAction(saveAsAct);
    menuFile->addAction(actShowFormSettings);
    menuFile->addSeparator();
    separatorAct = menuFile->addSeparator();
    QMenu* menuFileLast = menuFile->addMenu(tr("&Последние"));
    menuFile->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        menuFileLast->addAction(recentFileActs[i]);
    menuFile->addSeparator();
    menuFile->addAction(exitAct);
    updateRecentFileActions();
    menuBar()->addSeparator();

    QMenu* menuMacro = menuBar()->addMenu(tr("&Макро"));
    menuMacro->addAction(ActMacro);
    QMenu* menuCalc = menuBar()->addMenu(tr("&Расчеты"));
    menuCalc->addAction(actRGM);
    menuCalc->addAction(actOC);
    m_menuOpen = menuBar()->addMenu(tr("&Открыть") );
    menuBar()->addSeparator();
    QMenu* menuWindow = menuBar()->addMenu(tr("Окна"));
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
    QMenu* menuHelp = menuBar()->addMenu(tr("&Помощь"));
    menuHelp->addAction(aboutAct);

    //ToolBars
    QToolBar* toolbarFile = addToolBar(tr("Файл"));
    toolbarFile->addAction(newAct);
    toolbarFile->addAction(openAct);
    toolbarFile->addAction(saveAct);
    m_toolbarCalc = addToolBar(tr("Расчеты"));
    m_toolbarCalc->addAction(actRGM);
    m_toolbarCalc->addAction(actOC);

    //XZ
    createCalcLayout();
}
void MainWindow::setCurrentFile(const QString &fileName, const std::string Shablon)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings;
    QString _fileshabl = fileName;
    if (!Shablon.empty())
        _fileshabl.append(" <").append(Shablon).append(">");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(_fileshabl);
    files.prepend(_fileshabl);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        //QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));

        QStringList qslist = files[i].split(" ");
        std::string file = qslist[0].toStdString();
        std::string shabl = "";
        QString stripshabl = "";
        if (qslist.size() > 1)
        {
            shabl = qslist[1].toStdString();
            shabl.erase(shabl.begin());
            shabl.erase(shabl.end()-1);
            stripshabl = strippedName(shabl.c_str());
        }
        QString text;
        if (stripshabl.isEmpty())
            text = tr("&%1 %2").arg(i + 1).arg(qslist[0]);
        else
            text = tr("&%1 %2 %3").arg(i + 1).arg(qslist[0]).arg("<"+stripshabl+">");

        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}
QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
void MainWindow::Btn1_onClick(){

    QTableView* view = new QTableView;
    view->setWindowTitle("charttest");
    view->setWindowFlags(Qt::Tool);
    view->setModel(new ChartModel);
    view->show();

    //Show stock grid with nodes table
    /*
    QTableView* ptv = new QTableView();
    RModel* pmm = new RModel(nullptr,m_sp_qastra.get(),nullptr);
    const auto& forms = m_lstUIForms;
    auto it = forms.begin();
    std::advance(it,0);
    auto form  =*it;
    pmm->setForm(&form);
    pmm->populateDataFromRastr();
    ptv->setSortingEnabled(true);
    ptv->setModel(pmm);
    ptv->show();
    */
}
void MainWindow::Btn3_onClick(){
    //Show test ComboBox item at grid column

    QStandardItemModel* model = new QStandardItemModel(8, 2);
    QTableView* ptableView = new QTableView();;
    DelegateComboBox* delegate = new DelegateComboBox(this,"Item1,Item2,Item3");
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

bool MainWindow::maybeSave()
{
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

