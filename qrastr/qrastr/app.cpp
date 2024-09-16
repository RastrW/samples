#include <QString>
#include <QSettings>
#include <QMessageBox>
#include <QPluginLoader>
#include "app.h"
#include "common_qrastr.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>
#include "params.h"
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"
#include "plugin_interfaces.h"
#include "qastra.h"
#include "utils.h"
#include "UIForms.h"

App::App(int &argc, char **argv)
    :QApplication(argc, argv){
    upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
}
App::~App() {
}
bool App::event( QEvent *event ){
    const bool done = QApplication::event( event);
    return done;
}
bool App::notify(QObject* receiver, QEvent* event){
    bool done = false;
    try {
        done = QApplication::notify(receiver, event);
    }catch(std::exception& ex){
        exclog(ex);
        std::string str{fmt::format("std::exception: {}",ex.what())};
        assert(!str.c_str());
    }catch (...){
        exclog();
    }
    return done;
}
App::_cache_log::_cache_log( const spdlog::level::level_enum lev_in, std::string_view sv_in )
    : lev{lev_in}
    , str_log{sv_in}{
}
App::_cache_log& App::_cache_log::operator=(const App::_cache_log& cache_log){
    lev     = cache_log.lev;
    str_log = cache_log.str_log;
    return *this;
}
App::_cache_log& App::_cache_log::operator=(const App::_cache_log&& cache_log){
    operator=(cache_log);
    return *this;
}
App::_cache_log::_cache_log(const App::_cache_log& cache_log){
    operator=(cache_log);
}
App::_cache_log::_cache_log(const App::_cache_log&& cache_log){
    operator=(cache_log);
}
template <typename... Args>
void App::_v_cache_log::add( const spdlog::level::level_enum lev_in, const std::string_view sv_format, Args&&... args ){
    _cache_log cache_log{lev_in, fmt::format(sv_format, args...)};
    emplace_back(cache_log);
}
long App::readSettings(){ //it cache log messages to vector, because it called befor logger intialization
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
        }/*
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
        }*/
    }catch(const std::exception& ex){
        v_cache_log_.add( spdlog::level::err,"Exception: {} ", ex.what());
        return -4;
    }catch(...){
        v_cache_log_.add( spdlog::level::err,"Unknown exception.");
        return -5;
    }
    return 1;
}
long App::writeSettings(){
    QSettings settings(pchSettingsOrg_);
    //QSettings::IniFormat
    QString qstr = settings.fileName();
    //settings.setValue("pos", pos());
    //settings.setValue("size", size());
    return 1;
}
long App::init(){
    try{
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
    }catch(const std::exception& ex){
        exclog(ex);
        return -100;
    }catch(...){
        exclog();
        return -99;
    }
    return 1;
}
void App::loadPlugins(){
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

//        spdlog::info("loader.bindableObjectName: {}", loader.bindableObjectName().value().toStdString());
//        spdlog::info("loader.fileName: {}", loader.fileName().toStdString());
//        spdlog::info("loader.objectName: {}", loader.objectName().toStdString());
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
                    if(nullptr==rastr){
                        spdlog::error( "rastr==null" );
                        continue;
                    }
                    m_sp_qastra = std::make_shared<QAstra>();
                    m_sp_qastra->setRastr(rastr);

                    /*const bool myConnection = QObject::connect( m_sp_qastra.get(), SIGNAL( onRastrHint(const _hint_data&) ), this, SLOT( tst_onRastrHint(const _hint_data&) ) );
                    assert(myConnection == true);

                    m_sp_qastra->setRastr(rastr);
                    QDir::setCurrent(qdirData_.absolutePath());
                    m_sp_qastra->LoadFile( eLoadCode::RG_REPL, m_params.Get_on_start_load_file_rastr(), "" );
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
                    }
                    */
                }catch(const std::exception& ex){
                    exclog(ex);
                }catch(...){
                    exclog();
                }
                spdlog::info( "it is Rastr.test.finished");
            }
        }
    }
}
// form files are deployed in form catalog near qrastr.exe
long App::ReadForms(std::string str_path_forms){
    try{
        std::vector<std::string> forms = split(str_path_forms, ',');
        std::filesystem::path path_forms ("form");
        std::filesystem::path path_form_load;
#if(defined(_MSC_VER))
        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        for (std::string &form : forms){
            std::filesystem::path path_file_form = stringutils::utf8_decode(form);
            path_form_load =  path_forms / path_file_form;
            qDebug() << "read form from file : " << path_form_load.wstring();
    #else
            path_forms_load = str_path_to_file_forms;
            qDebug() << "read form from file : " << path_forms_load.c_str();
    #endif
            CUIFormsCollection* CUIFormsCollection_ = new CUIFormsCollection ;
            if (path_form_load.extension() == ".fm")
                *CUIFormsCollection_ = CUIFormCollectionSerializerBinary(path_form_load).Deserialize();
            else
                *CUIFormsCollection_ = CUIFormCollectionSerializerJson(path_form_load).Deserialize();
            for(const  CUIForm& uiform : CUIFormsCollection_->Forms()){
                upCUIFormsCollection_->Forms().emplace_back(uiform);
            }
        }
        qDebug() << "Thats all forms.\n" ;
    }catch(const std::exception& ex){
        exclog(ex);
        return -1;
    }catch(...){
        exclog();
        return -2;
    }
    return 1;
}
std::list<CUIForm>& App::GetForms() const {
    assert(nullptr!=upCUIFormsCollection_);
    return upCUIFormsCollection_->Forms();
}
/*
CUIForm CRastrHlp::GetUIForm(size_t n_form_indx){
    auto it = upCUIFormsCollection_->Forms().begin();
    std::advance(it,n_form_indx);
    auto form  =*it;

    return form;
}

int CRastrHlp::GetFormData(int n_form_indx){
    if(n_form_indx<0)
        return -1;
    auto it = upCUIFormsCollection_->Forms().begin();
    std::advance(it,n_form_indx);
    auto form  =*it;

    return 1;
*/

long App::start(){
    try{
        long n_res =0;
        QDir::setCurrent(qdirData_.absolutePath());
        loadPlugins();
        if(nullptr!=m_sp_qastra){
            m_sp_qastra->LoadFile( eLoadCode::RG_REPL, m_params.Get_on_start_load_file_rastr(), "" );
            if(n_res<0){
                spdlog::error("{} =LoadFile()", n_res);
                QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                QString("error: %1 wheh read file : %2").arg(n_res).arg(m_params.Get_on_start_load_file_rastr().c_str())
                               );
                mb.exec();
            }
        }
        spdlog::info("ReadForms: {}", m_params.Get_on_start_load_file_forms() );
        n_res = ReadForms(m_params.Get_on_start_load_file_forms());
        if(n_res<0){
            spdlog::error("{} =ReadForms()", n_res);
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString("error: %1 wheh read file : %2").arg(n_res).arg(m_params.Get_on_start_load_file_forms().c_str())
                           );
            mb.exec();
        }

    }catch(const std::exception& ex){
        exclog(ex);
        return -200;
    }catch(...){
        exclog();
        return -199;
    }
    return 1;
}

