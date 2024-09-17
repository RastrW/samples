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


MainWindow::MainWindow(){
    auto logg = std::make_shared<spdlog::logger>( "qrastr" );
    spdlog::set_default_logger(logg);
    SetConsoleOutputCP(CP_UTF8);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    logg->sinks().push_back(console_sink);
    int n_res = readSettings();
    const bool bl_res = QDir::setCurrent(qdirData_.path()); assert(bl_res==true);
    std::filesystem::path path_log{ qdirData_.absolutePath().toStdString() };
    path_log /= L"qrastr_log.txt";
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( path_log.c_str(), 1024*1024*1, 3);
    std::vector<spdlog::sink_ptr> sinks{ console_sink, rotating_sink };
    logg->sinks().push_back(rotating_sink);
    spdlog::info( "ReadSetting: {}", n_res );
    spdlog::info( "Log: {}", path_log.generic_u8string() );
    m_workspace = new QMdiArea;
    setCentralWidget(m_workspace);
    connect(m_workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateMenus()));
    m_windowMapper = new QSignalMapper(this);
    connect(m_windowMapper, SIGNAL(mappedWidget(QWidget *)), SLOT(setActiveSubWindow(QWidget *)));
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();
    setForms();
    setWindowTitle(tr("~qrastr~"));
    //int nRes = test();
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton, true);
    m_DockManager = new ads::CDockManager(this);
    QObject::connect(m_DockManager, &ads::CDockManager::focusedDockWidgetChanged
                                  , [] (ads::CDockWidget* old, ads::CDockWidget* now) {
        static int Count = 0;
        qDebug() << Count++ << " CDockManager::focusedDockWidgetChanged old: " << (old ? old->objectName() : "-") << " now: " << now->objectName() << " visible: " << now->isVisible();
        now->widget()->setFocus();
    });
    //int n_res = readSettings();
    McrWnd* pMcrWnd = new McrWnd( this, McrWnd::_en_role::global_protocol );
    if(false){
        QDockWidget *dock = new QDockWidget( "protocol", this);
        dock->setWidget(pMcrWnd);
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea | Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }else{
        static int i = 0;
        auto dw = new ads::CDockWidget( "protocol", this);
        dw->setWidget(pMcrWnd);
        int f = ads::CDockWidget::CustomCloseHandling;
        dw->setFeature( static_cast<ads::CDockWidget::DockWidgetFeature>(f), true);
        //auto area = m_DockManager->addDockWidgetTab(ads::NoDockWidgetArea, dw);
        auto container = m_DockManager->addDockWidgetFloating(dw);
        container->move(QPoint(2100, 20));
        container->resize(1200,800);
    }
    auto qt_sink = std::make_shared<spdlog::sinks::qt_sink_mt>(pMcrWnd, "onQStringAppendProtocol");
    logg->sinks().push_back(qt_sink);
    pMcrWnd->show();
    
    loadPlugins();


}
MainWindow::~MainWindow(){
}
int MainWindow::readSettings(){ //it cache log messages to vector, because it called befor logger intialization
    try{
        QSettings settings(pchSettingsOrg_);
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 800)).toSize();
        //move(pos);
        //resize(size);

        int nRes = 0;
        QString qstr_curr_path = QDir::currentPath();
        std::string str_path_2_conf = "undef";
#if(defined(COMPILE_WIN))
        //str_path_2_conf = R"(C:\projects\git_web\samples\qrastr\qrastr\appsettings.json)";
        //str_path_2_conf = R"(appsettings.json)";
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+pchSettingsDirData_ + "/appsettings.json";
        //str_path_2_conf = R"(..\..\appsettings.json)";
#else
        str_path_2_conf = R"(/home/ustas/projects/git_web/samples/qrastr/qrastr/appsettings.json)";
        //QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QString("In lin not implemented!") );  mb.exec();
#endif
        QFileInfo qfi(str_path_2_conf.c_str());
        qdirData_ = qfi.dir();
        const bool bl_res = QDir::setCurrent(qdirData_.path());
        if(bl_res == true){
            v_cache_log_.add(spdlog::level::info, "Set DataDir: {}", qdirData_.path().toStdString());
        }else{
            v_cache_log_.add(spdlog::level::err, "Can't set DataDir: {}", qdirData_.path().toStdString());
        }
        nRes = m_params.ReadJsonFile(str_path_2_conf);
        if(nRes<0){
            QMessageBox mb;
            // так лучше не делать ,смешение строк qt и std это боль.
            QString qstr = QObject::tr("Can't load on_start_file: ");
            std::string qstr_fmt = qstr.toUtf8().constData(); //  qstr.toStdString(); !!not worked!!
            std::string ss = fmt::format( "{}{} ", qstr_fmt.c_str(), m_params.Get_on_start_load_file_rastr());
            QString str = QString::fromUtf8(ss.c_str());
            mb.setText(str);
            v_cache_log_.add( spdlog::level::err, "{} ReadJsonFile {}", nRes, str.toStdString());
            mb.exec();
            return -1;
        }
        v_cache_log_.add(spdlog::level::info, "**************************************************************************");
        up_rastr_ = std::make_unique<CRastrHlp>();
        nRes = up_rastr_->CreateRastr();
        v_cache_log_.add(spdlog::level::info, "CreateRastr()");
        if(nRes < 0){
            v_cache_log_.add( spdlog::level::err, "{} CreateRastr()", nRes);
            return -1;
        }
        v_cache_log_.add(spdlog::level::info, "**************************************************************************");
        v_cache_log_.add(spdlog::level::info, "Load: {}", m_params.Get_on_start_load_file_rastr());
        nRes = up_rastr_->Load(m_params.Get_on_start_load_file_rastr());
        if(nRes < 0){
            v_cache_log_.add( spdlog::level::err, "{} Load(...)", nRes);
            return -1;
        }
        v_cache_log_.add(spdlog::level::info, "ReadForms : {}", m_params.Get_on_start_load_file_forms());
        nRes = up_rastr_->ReadForms(m_params.Get_on_start_load_file_forms());
        if(nRes<0){
            v_cache_log_.add( spdlog::level::err, "{} ReadForms()", nRes);
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString("error: %1 wheh read file : %2").arg(nRes).arg(m_params.Get_on_start_load_file_forms().c_str())
                           );
            mb.exec();
            return -1;
        }
    }catch(const std::exception& ex){
        v_cache_log_.add( spdlog::level::err,"Exception: {} ", ex.what());
        return -4;
    }catch(...){
        v_cache_log_.add( spdlog::level::err,"Unknown exception.");
        return -5;
    }
    return 1;
}
int MainWindow::writeSettings(){
    QSettings settings(pchSettingsOrg_);
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
void MainWindow::loadPlugins(){
    //const auto staticInstances = QPluginLoader::staticInstances();
    //for (QObject *plugin : staticInstances){
    //    spdlog::log(spdlog::level::info, "Load static plugin: {}", plugin->objectName().toStdString());
    //}
    QDir pluginsDir{QDir{QCoreApplication::applicationDirPath()}};
    pluginsDir.cd("plugins");
    spdlog::info("Plugins dir: {}", pluginsDir.absolutePath().toStdString());
#if(COMPILE_WIN)
    const auto entryList = pluginsDir.entryList(QStringList() << "*.dll", QDir::Files);
#else
    const auto entryList = pluginsDir.entryList(QDir::Files);
#endif
    for( const QString &fileName : entryList ){
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        spdlog::info("loader.bindableObjectName: {}", loader.bindableObjectName().value().toStdString());
        spdlog::info("loader.fileName: {}", loader.fileName().toStdString());
        spdlog::info("loader.objectName: {}", loader.objectName().toStdString());
        QObject *plugin = loader.instance();
        if(plugin){
            spdlog::info( "Load dynamic plugin {}/{} : {}", pluginsDir.absolutePath().toStdString(), fileName.toStdString(), plugin->objectName().toStdString());
            auto iRastr = qobject_cast<InterfaceRastr *>(plugin);
            if(iRastr){
                try{
                    spdlog::info( "it is Rastr" );
                    const std::shared_ptr<spdlog::logger> sp_logger = spdlog::default_logger();
                    iRastr->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainRastr> rastr = iRastr->getIPlainRastrPtr(); // Destroyable rastr{ iRastr };
                    m_sp_qastra = std::make_unique<QAstra>();
                    //QMetaObject::Connection  ccc = connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), this, SLOT( tst_onRastrHint(const _hint_data&) ) );
                    //static const bool myConnection =connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), this, SLOT( tst_onRastrHint(const _hint_data&) ) );
                    const bool myConnection = QObject::connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), this, SLOT( tst_onRastrHint(const _hint_data&) ) );
                    assert(myConnection == true);

                    m_sp_qastra->setRastr(rastr);
                    QDir::setCurrent(qdirData_.absolutePath());
                    m_sp_qastra->LoadFile( eLoadCode::RG_REPL, m_params.Get_on_start_load_file_rastr(), "" );
                    bool bHints = false;
                    if(bHints){
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

                    if(false){
                        /*
                        EventSink sink;
                        IRastrResultVerify(rastr->SubscribeEvents(&sink));
                        //std::filesystem::path path_file{LR"(C:\Users\ustas\Documents\RastrWin3\test-rastr\cx195.rg2)"};
                        //std::filesystem::path path_file{LR"(C:\Temp\Новая папка\mega2_ur_.rg2)"};
                        std::filesystem::path path_file{LR"(C:\Temp\Новая папка\cx195.rg2)"};
                        IRastrResultVerify loadresult{  //IPlainRastrResult* pip { rastr->Load(eLoadCode::RG_REPL, path_file.generic_u8string(), "") };
                                rastr->Load(
                                eLoadCode::RG_REPL,
                                stringutils::acp_encode(path_file.generic_u8string()),
                                //stringutils::acp_encode(R"(C:\Users\ustas\Documents\RastrWin3\SHABLON\режим.rg2)")
                                ""
                            )
                        };
                        spdlog::info( "Rastr.Load {}", path_file.generic_u8string());
                        IRastrPayload rgmresult{ rastr->Rgm("p1") };
                        spdlog::info( "{} = Rgm(...) ", static_cast<std::underlying_type<eASTCode>::type>(rgmresult.Value()) );
                        if( rgmresult.Value() == eASTCode::AST_OK ){
                            spdlog::info( "Rgm ok!");
                        }else{
                            spdlog::error( "Rgm fail!");
                        }

                        IRastrTablesPtr tablesx{ rastr->Tables() };
                        IRastrPayload tablecount{ tablesx->Count() };
                        spdlog::info("tablecount: {}", tablecount.Value() );
                        IRastrTablePtr nodes{ tablesx->Item("node") };

                        IRastrPayload tablesize{ nodes->Size() };
                        spdlog::info("node.tablesize: {}",tablesize.Value());
                        IRastrPayload tablename{ nodes->Name() };
                        spdlog::info("node.tablename: {}",tablename.Value() );
                        IRastrObjectPtr<IPlainRastrColumns> nodecolumns{ nodes->Columns() };
                        IRastrColumnPtr ny{ nodecolumns->Item("ny") };
                        IRastrColumnPtr name{ nodecolumns->Item("name") };
                        IRastrColumnPtr v{ nodecolumns->Item("vras") };
                        IRastrColumnPtr delta{ nodecolumns->Item("delta") };
                        for (long index{ 0 }; index < tablesize.Value(); index++) {
                            IRastrVariantPtr Varny{ ny->Value(index) };
                            IRastrVariantPtr Varname{ name->Value(index) };
                            IRastrVariantPtr Varv{ v->Value(index) };
                            IRastrVariantPtr Vardelta{ delta->Value(index) };

                            IRastrPayload nyvalue{ Varny->Long() };
                            IRastrPayload namevalue{ Varname->String() };
                            IRastrPayload vvalue{ Varv->Double() };
                            IRastrPayload deltavalue{ Vardelta->Double() };
                            spdlog::info( "ny: {:10} name: {:15} vras: {:3.2f} delta: {:2.3f}"
                                , nyvalue.Value()
                                , stringutils::acp_decode(namevalue.Value())
                                , vvalue.Value()
                                , deltavalue.Value()
                            );
                        }
                        IRastrResultVerify selectionresult{ nodes->SetSelection("uhom>330") };
                        IRastrObjectPtr<IPlainRastrDataset> dataset{ nodes->Dataset("ny, name, uhom, 62,")};
                        const long datasize{ IRastrPayload(dataset->Count()).Value() };

                        for (long colindex = 0; colindex < datasize; colindex++)
                        {
                            IRastrObjectPtr<IPlainRastrDatasetColumn> datacolumn{ dataset->Item(colindex) };
                            spdlog::info( "col_name: {:10} col_type: {}"
                                , IRastrPayload(datacolumn->Name()).Value()
                                , static_cast<std::underlying_type<ePropType>::type>(IRastrPayload(datacolumn->Type()).Value())
                            );
                            const long columnsize{ IRastrPayload(datacolumn->Count()).Value() };
                            for (long rowindex = 0; rowindex < columnsize; rowindex++)
                            {
                                IRastrObjectPtr<IPlainRastrDatasetValue> indexedvalue{ datacolumn->Item(rowindex) };
                                IRastrVariantPtr value{ indexedvalue->Value() };
                                spdlog::info(" indx: {:5} str: {:40}"
                                    , IRastrPayload(indexedvalue->Index()).Value()
                                    , stringutils::acp_decode(IRastrPayload(value->String()).Value())
                                );
                            }
                        }
                        IRastrObjectPtr dcol{ dataset->Item("ny") };
                        // на выходе из скопа врапперы делают Destroy в хипе астры
                        IRastrResultVerify(rastr->UnsubscribeEvents(&sink));
                        */
                    }//if(false)

                }catch(const std::exception& ex){
                    exclog( ex );
                }
                spdlog::info( "it is Rastr.test.finished");
            }
        }
    }
}
void MainWindow::logCacheFlush(){
    for( const auto& cache_log : v_cache_log_){
        spdlog::log(cache_log.lev, cache_log.str_log);
    }
    v_cache_log_.clear();
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


void MainWindow::setForms(){ // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    int i = 0;
    QMap<QString,QMenu *> map_menu;
    //QMap<QString, QMenu>::iterator it ;
    auto forms = up_rastr_->GetForms();
    for(const auto& j_form : forms){
        std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
        if (!str_MenuPath.empty() && str_MenuPath.at(0) == '_')
            continue;
        QString qstr_MenuPath = str_MenuPath.c_str();
        if (!str_MenuPath.empty() && !map_menu.contains(qstr_MenuPath))
            map_menu.insert(qstr_MenuPath,m_openMenu->addMenu(str_MenuPath.c_str()));
    }
    for(const auto& j_form : forms){
        std::string str_Name = stringutils::cp1251ToUtf8(j_form.Name());
        std::string str_TableName = j_form.TableName();
        std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
        QString qstr_MenuPath = str_MenuPath.c_str();
        QMenu* cur_menu = m_openMenu;
        if (map_menu.contains(qstr_MenuPath))
            cur_menu = map_menu[qstr_MenuPath];
        if (!str_Name.empty() && str_Name.at(0) != '_'){
            QAction* p_actn = cur_menu->addAction(str_Name.c_str());
            p_actn->setData(i);
        }
        i++;
    }
    connect( m_openMenu, SIGNAL(triggered(QAction *)), this, SLOT(onOpenForm(QAction *)), Qt::UniqueConnection);
}
void MainWindow::closeEvent(QCloseEvent *event){
    QMainWindow::closeEvent(event);
    writeSettings();
    spdlog::warn( "On closeEvent");
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
    //m_DockManager->close();
}
void MainWindow::newFile(){
#if(!defined(QICSGRID_NO))
    //MdiChild *child = createMdiChild(  j_forms_[0] );
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
        int nRes = 0;
        std::string f = fileName.toUtf8().constData(); //  it works!!!
        nRes = up_rastr_->Load( f);
        //nRes = Load( id_rastr_, fileName.toStdString().c_str(), "");
        if(nRes>0){
            std::string str_msg = fmt::format( "{}: {}", tr("File loaded").toStdString(), fileName.toStdString());
            statusBar()->showMessage( str_msg.c_str(), 2000 );
            cur_file = f;
            emit file_loaded(*up_rastr_.get());
        } else {
            std::string str_msg = fmt::format( "{}: {}", tr("File not loaded").toStdString(), fileName.toStdString());
            QMessageBox msgBox;
            msgBox.critical( this, tr("File not loaded"), str_msg.c_str() );
        }

        m_sp_qastra->LoadFile(eLoadCode::RG_REPL, fileName.toStdString(),"");
    }
}
void MainWindow::save(){
#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif//#if(!defined(QICSGRID_NO))
    if (!cur_file.empty()) {
        int nRes = 0;
        std::string f = cur_file;
        nRes = up_rastr_->Save( f);
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
        nRes = up_rastr_->Save( f);
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
void MainWindow::cut(){
#if(!defined(QICSGRID_NO))
    activeMdiChild()->cut();
#endif//#if(!defined(QICSGRID_NO))
}
void MainWindow::copy(){
#if(!defined(QICSGRID_NO))
    activeMdiChild()->copy();
#endif//#if(!defined(QICSGRID_NO))
}
void MainWindow::paste(){
#if(!defined(QICSGRID_NO))
    activeMdiChild()->paste();
#endif// #if(!defined(QICSGRID_NO))
}
void MainWindow::insertRow(){
#if(!defined(QICSGRID_NO))
    int rowIndex = activeMdiChild()->currentCell()->rowIndex();
    if (rowIndex < 0)
        return;
    activeMdiChild()->insertRow( rowIndex );
#endif//#if(!defined(QICSGRID_NO))
}
void MainWindow::deleteRow(){
#if(!defined(QICSGRID_NO))
     // DO NOT WORK... WHY ?
    const QicsCell * cell = activeMdiChild()->currentCell();
    activeMdiChild()->deleteRow( cell->rowIndex() );
#endif
}
void MainWindow::insertCol(){
#if(!defined(QICSGRID_NO))
    int colIndex = activeMdiChild()->currentCell() ->columnIndex();
    if (colIndex < 0)
        return;
    activeMdiChild()->insertColumn( colIndex );
#endif //#if(!defined(QICSGRID_NO))
}
void MainWindow::deleteCol(){
#if(!defined(QICSGRID_NO))
    const QicsCell * cell = activeMdiChild()->currentCell();
    activeMdiChild()->deleteColumn( cell->columnIndex() );
#endif // #if(!defined(QICSGRID_NO))
}
void MainWindow::rgm_wrap(){
    long res = Rgm(id_rastr_,"");
    std::string str_msg = "";
    if (res == 0){
        str_msg = "Расчет режима выполнен успешно";
        spdlog::info("{}", str_msg);
    }else{
        str_msg = "Расчет режима завершился аварийно!";
        spdlog::error("{} : {}", res, str_msg);
    }
    statusBar()->showMessage( str_msg.c_str(), 0 );

    m_sp_qastra->Rgm("");

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
    const auto forms = up_rastr_->GetForms();
    auto it = forms.begin();
    std::advance(it,n_indx);
    auto form  =*it;
    qDebug() << "\n Open form:" + form.Name();
    spdlog::info( "Create tab [{}]", stringutils::cp1251ToUtf8(form.Name()) );
    RtabWidget *prtw = new RtabWidget(m_sp_qastra.get(),form,this);
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
    emit rm_change(_t_name,index,value);
}
void MainWindow::ondataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value){
    emit rm_change(_t_name,_col_name,_row,_value);
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
void MainWindow::updateWindowMenu(){
    m_windowMenu->clear();
    m_windowMenu->addAction(m_closeAct);
    m_windowMenu->addAction(m_closeAllAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_tileAct);
    m_windowMenu->addAction(m_cascadeAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_nextAct);
    m_windowMenu->addAction(m_previousAct);
    m_windowMenu->addAction(m_separatorAct);
    QList<QMdiSubWindow *> windows = m_workspace->subWindowList();
    m_separatorAct->setVisible(!windows.isEmpty());
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
}
void MainWindow::setActiveSubWindow(QWidget *window){
    m_workspace->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}
void MainWindow::createActions(){
    m_newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    m_newAct->setShortcut(tr("Ctrl+N"));
    m_newAct->setStatusTip(tr("Create a new file"));
    connect(m_newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    m_openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    m_openAct->setShortcut(tr("Ctrl+O"));
    m_openAct->setStatusTip(tr("Open an existing file"));
    connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));
    m_saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    m_saveAct->setShortcut(tr("Ctrl+S"));
    m_saveAct->setStatusTip(tr("Save the document to disk"));
    connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));
    m_saveAsAct = new QAction(tr("Save &As..."), this);
    m_saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcut(tr("Ctrl+Q"));
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    m_cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    m_cutAct->setShortcut(tr("Ctrl+X"));
    m_cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(m_cutAct, SIGNAL(triggered()), this, SLOT(cut()));
    m_copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    m_copyAct->setShortcut(tr("Ctrl+C"));
    m_copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(m_copyAct, SIGNAL(triggered()), this, SLOT(copy()));
    m_pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    m_pasteAct->setShortcut(tr("Ctrl+V"));
    m_pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(m_pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    m_insertRowAct = new QAction( QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr( "&Insert Row" ), this );
    m_insertRowAct->setShortcut( tr( "Ctrl+I" ) );
    connect( m_insertRowAct, SIGNAL( triggered() ), this, SLOT( insertRow() ) );

    m_deleteRowAct = new QAction( QIcon(":/images/Rastr3_grid_delrow_16x16.png"), tr("&Delete Row" ), this );
    m_deleteRowAct->setShortcut( tr( "Ctrl+D" ) );
    connect( m_deleteRowAct, SIGNAL( triggered() ), this, SLOT( deleteRow() ) );

    m_insertColAct = new QAction( tr( "&Insert Column" ), this );
    m_insertColAct->setShortcut( tr( "Ctrl+K" ) );
    connect( m_insertColAct, SIGNAL( triggered() ), this, SLOT( insertCol() ) );

    m_deleteColAct = new QAction( tr( "&Delete Column" ), this );
    m_deleteColAct->setShortcut( tr( "Ctrl+L" ) );
    connect( m_deleteColAct, SIGNAL( triggered() ), this, SLOT( deleteCol() ) );

    m_closeAct = new QAction(tr("Cl&ose"), this);
    m_closeAct->setShortcut(tr("Ctrl+F4"));
    m_closeAct->setStatusTip(tr("Close the active window"));
    connect(m_closeAct, SIGNAL(triggered()),
            m_workspace, SLOT(closeActiveSubWindow()));
    m_closeAllAct = new QAction(tr("Close &All"), this);
    m_closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(m_closeAllAct, SIGNAL(triggered()),
            m_workspace, SLOT(closeAllSubWindows()));
    m_tileAct = new QAction(tr("&Tile"), this);
    m_tileAct->setStatusTip(tr("Tile the windows"));
    connect(m_tileAct, SIGNAL(triggered()), m_workspace, SLOT(tileSubWindows()));
    m_cascadeAct = new QAction(tr("&Cascade"), this);
    m_cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(m_cascadeAct, SIGNAL(triggered()), m_workspace, SLOT(cascadeSubWindows()));
    m_nextAct = new QAction(tr("Ne&xt"), this);
    m_nextAct->setShortcut(tr("Ctrl+F6"));
    m_nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(m_nextAct, SIGNAL(triggered()),
            m_workspace, SLOT(activateNextSubWindow()));
    m_previousAct = new QAction(tr("Pre&vious"), this);
    m_previousAct->setShortcut(tr("Ctrl+Shift+F6"));
    m_previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(m_previousAct, SIGNAL(triggered()),
            m_workspace, SLOT(activatePreviousSubWindow()));
    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_SortAscAct = new QAction(QIcon(":/images/sortasc.png"),tr("&SortAsc"), this);
    m_SortAscAct->setShortcut(tr("Ctrl+Up"));
    m_SortAscAct->setStatusTip(tr("Sort Ascending (Ctrl+Up)"));
    connect(m_SortAscAct, SIGNAL(triggered()), this, SLOT(sortAscending()));

    m_SortDescAct = new QAction(QIcon(":/images/sortdesc.png"),tr("&SortDesc"), this);
    m_SortDescAct->setShortcut(tr("Ctrl+Down"));
    m_SortDescAct->setStatusTip(tr("Sort Descending (Ctrl+Down)"));
    connect(m_SortDescAct, SIGNAL(triggered()), this, SLOT(sortDescending()));

    m_RGMAct = new QAction(QIcon(":/images/Rastr3_rgm_16x16.png"),tr("&rgm"), this);
    m_RGMAct->setShortcut(tr("F5"));
    m_RGMAct->setStatusTip(tr("Calc rgm"));
    //connect(m_RGMAct, SIGNAL(triggered()), this, SLOT(Rgm(id_rastr_,"")));
    connect(m_RGMAct, SIGNAL(triggered()), this, SLOT(rgm_wrap()));

    m_ActMacro = new QAction(QIcon(":/images/cut.png"),tr("&macro"), this);
    m_ActMacro->setShortcut(tr("F11"));
    m_ActMacro->setStatusTip(tr("Run macro"));
    connect(m_ActMacro, SIGNAL(triggered()), this, SLOT(onDlgMcr()));
}
void MainWindow::createMenus(){
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_cutAct);
    m_editMenu->addAction(m_copyAct);
    m_editMenu->addAction(m_pasteAct);
    m_editMenu->addAction( m_insertRowAct );
    m_editMenu->addAction( m_deleteRowAct );
    m_editMenu->addAction( m_insertColAct );
    m_editMenu->addAction( m_deleteColAct );
   // m_viewMenu = menuBar()->addMenu(tr("&View"));
    //m_editMenu->addAction(m_SortAscAct);

    m_CalcMenu = menuBar()->addMenu(tr("&Calc"));
    m_CalcMenu->addAction(m_RGMAct);

    m_CalcMenu = menuBar()->addMenu(tr("&Macro"));
    m_CalcMenu->addAction(m_ActMacro);

    m_openMenu = menuBar()->addMenu(tr("&Open") );
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
    menuBar()->addSeparator();
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}
void MainWindow::createToolBars(){
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newAct);
    m_fileToolBar->addAction(m_openAct);
    m_fileToolBar->addAction(m_saveAct);
    m_editToolBar = addToolBar(tr("Edit"));
    m_editToolBar->addAction(m_cutAct);
    m_editToolBar->addAction(m_copyAct);
    m_editToolBar->addAction(m_pasteAct);
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->addAction(m_SortAscAct);
    m_viewToolBar->addAction(m_SortDescAct);
    m_calcToolBar = addToolBar(tr("Calc"));
    m_calcToolBar->addAction(m_RGMAct);
    createCalcLayout();
}


void MainWindow::Btn1_onClick()
{
    //Show stock grid with nodes table

    QTableView* ptv = new QTableView();
    CRastrHlp rhlp;
    //*up_rastr_.get()
    RModel* pmm = new RModel(nullptr,m_sp_qastra.get());
    const auto forms = up_rastr_->GetForms();
    auto it = forms.begin();
    std::advance(it,0);
    auto form  =*it;
    pmm->setForm(&form);
    pmm->populateDataFromRastr();
    ptv->setSortingEnabled(true);
    ptv->setModel(pmm);
    ptv->show();
}
void MainWindow::Btn3_onClick()
{
    //Show test ComboBox item at grid column

    QStandardItemModel* model = new QStandardItemModel(4, 2);
    QTableView* ptableView = new QTableView();;

    ComboBoxDelegate* delegate = new ComboBoxDelegate(this,"Item1,Item2,Item3");
    //tableView.setItemDelegate(&delegate);
    ptableView->setItemDelegateForColumn(1, delegate); // Column 0 can take any value, column 1 can only take values up to 8.

    for (int row = 0; row < 4; ++row)
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
void MainWindow::createCalcLayout()
{
    // набор вложенных виджетов - кнопок
    QPushButton *btn1 = new QPushButton("Stock grid");
    QPushButton *btn2 = new QPushButton("Button 2");
    QPushButton *btn3 = new QPushButton("Tst ComboBoxDelegate");
    //QPushButton *btn4 = new QPushButton("Button 4");

    connect(btn1,&QPushButton::clicked,this, &MainWindow::Btn1_onClick);
    connect(btn2,&QPushButton::clicked,this, &MainWindow::onButton2Click);
    connect(btn3,&QPushButton::clicked,this, &MainWindow::Btn3_onClick);

    QWidget* widget = new QWidget;
    widget -> setWindowTitle("Functions");
    m_ActionsLayout = new QHBoxLayout(widget);
    m_ActionsLayout->addWidget(btn1);
//    m_ActionsLayout->addWidget(btn2);
    m_ActionsLayout->addWidget(btn3);
   // m_ActionsLayout->addWidget(btn4);
    m_calcToolBar->addWidget(widget);
    //view->setSortingEnabled(true);
}
void MainWindow::createStatusBar()
{
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

