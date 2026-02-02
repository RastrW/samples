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
#include <astra/IPlainRastrWrappers.h>
//#include "IPlainRastrWrappers.h"
//#include "C:\Projects\tfs\rastr\RastrWin\KC\IPlainTI.h"
#include "plugins/rastr/plugin_interfaces.h"
#include "plugins/ti/plugin_ti_interfaces.h"
#include "plugins/barsmdp/plugin_barsmdp_interfaces.h"
#include "qastra.h"
#include "qti.h"
#include "qbarsmdp.h"
#include "utils.h"
//#include "UIForms.h"
//#include <astra\UIForms.h>
#include "astra_headers/UIForms.h"

App::App(int &argc, char **argv)
    :QApplication(argc, argv){
    upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
}

App::~App() {
    Params::Destruct();
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
        const std::string str{fmt::format("std::exception: {}",ex.what())};
        //spdlog::error("ERROR: {}", ex.what());
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
        Params::Construct();
        QSettings settings(Params::pch_org_qrastr_);
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 800)).toSize();
        //move(pos);
        //resize(size);

        int nRes = 0;
        QString qstr_curr_path = QDir::currentPath();
        std::string str_path_2_conf = "undef";
#if(defined(COMPILE_WIN))
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+Params::pch_dir_data_ +"/"+ Params::pch_fname_appsettings;
#else
        //str_path_2_conf = R"(/home/ustas/projects/git_web/samples/qrastr/qrastr/appsettings.json)";
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+Params::pch_dir_data_ +"/"+ Params::pch_fname_appsettings;
       // QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QString("In lin not implemented!") );  mb.exec();
#endif
        QFileInfo fi_appsettings(str_path_2_conf.c_str());
        Params* const p_params = Params::GetInstance();
        if(p_params!=nullptr){
            p_params->setDirData(fi_appsettings.dir());
            const bool bl_res = QDir::setCurrent(p_params->getDirData().path());
            if(bl_res == true){
                v_cache_log_.add(spdlog::level::info, "Set DataDir: {}", p_params->getDirData().path().toStdString());
            }else{
                v_cache_log_.add(spdlog::level::err, "Can't set DataDir: {}", p_params->getDirData().path().toStdString());
            }
            nRes = p_params->readJsonFile(str_path_2_conf);
            if(nRes < 0){

                QMessageBox mb;
                // так лучше не делать ,смешение строк qt и std это боль.
                QString qstr = QObject::tr("Can't load on_start_file: ");
                std::string qstr_fmt = qstr.toUtf8().constData(); //  qstr.toStdString(); !!not worked!!
                //std::string ss = fmt::format( "{}{} ", qstr_fmt.c_str(), p_params->Get_on_start_load_file_rastr());
                std::string ss = "error in files load";
                QString str = QString::fromUtf8(ss.c_str());
                mb.setText(str);
                v_cache_log_.add( spdlog::level::err, "{} ReadJsonFile {}", nRes, str.toStdString());
                mb.exec();
                return -1;
            }
            p_params->setFileAppsettings(str_path_2_conf);
            v_cache_log_.add(spdlog::level::info, "ReadTemplates: {}", p_params->getDirSHABLON().absolutePath().toStdString());
            nRes = Params::GetInstance()->readTemplates( p_params->getDirSHABLON().absolutePath().toStdString() );
            assert(nRes>0);
            if(nRes < 0){
                v_cache_log_.add(spdlog::level::err, "Error while read: {}", nRes);
            }

            const std::filesystem::path path_dirforms = p_params->getDirData().canonicalPath().toStdString()+"//form//";
            v_cache_log_.add(spdlog::level::info, "ReadForms: {}", path_dirforms.string());
            nRes = Params::GetInstance()->readFormsExists( path_dirforms );
            assert(nRes>0);
            if(nRes < 0){
                v_cache_log_.add(spdlog::level::err, "Error while read existed forms: {}", nRes);
            }
            nRes = Params::GetInstance()->readForms( path_dirforms );
            assert(nRes>0);
            if(nRes < 0){
                v_cache_log_.add(spdlog::level::err, "Error while read: {}", nRes);
            }

        }else{
            v_cache_log_.add(spdlog::level::err, "Can't create singleton Params");
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

long App::writeSettings(){
    QSettings settings(Params::pch_org_qrastr_);
    //QSettings::IniFormat
    QString qstr = settings.fileName();
    return 1;
}

long App::init(){
    qInfo() << "=== App::init() START ===";
    try{
        auto logg = std::make_shared<spdlog::logger>( "qrastr" );
        spdlog::set_default_logger(logg);
#if(defined(_MSC_VER))
        SetConsoleOutputCP(CP_UTF8);
#endif
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logg->sinks().push_back(console_sink);
        int n_res = readSettings();
        //const bool bl_res = QDir::setCurrent(qdirData_.path()); assert(bl_res==true);
        const bool bl_res = QDir::setCurrent(Params::GetInstance()->getDirData().path()); assert(bl_res==true);
        //std::filesystem::path path_log{ qdirData_.absolutePath().toStdString() };
        fs::path path_log{ Params::GetInstance()->getDirData().absolutePath().toStdString() };
        path_log /= L"qrastr_log.txt";
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( path_log.string(), 1024*1024*1, 3);
        //auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( "sdfdsf", 1024*1024*1, 3);
        //auto rotating_sink =            spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
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
    qInfo() << "=== App::init() END ===";
    return 1;
}

void App::loadPlugins(){
    QDir pluginsDir{QDir{QCoreApplication::applicationDirPath()}};
    pluginsDir.cd("plugins");
    spdlog::info("Plugins dir: {}", pluginsDir.absolutePath().toStdString());

    //путь к плагинам в библиотечные пути
    QCoreApplication::addLibraryPath(pluginsDir.absolutePath());

#if(COMPILE_WIN)
    auto entryList = pluginsDir.entryList(QStringList() << "*.dll", QDir::Files);
#else
    auto entryList = pluginsDir.entryList(QDir::Files);
#endif

// move rastr.dll at top
#if(COMPILE_WIN)
    int ind_rastr = entryList.indexOf("rastr.dll");
#else
    int ind_rastr = entryList.indexOf("librastr.so");
#endif

    if(ind_rastr >= 0) {  // Проверка на случай отсутствия файла
        auto item = entryList.takeAt(ind_rastr);
        entryList.insert(0, item);
    }

    for(const QString &fileName : entryList){
        QString fullPath = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader loader(fullPath);

        // Проверка метаданных перед загрузкой
        QJsonObject metaData = loader.metaData();
        if(metaData.isEmpty()) {
            spdlog::warn("{} is not a valid Qt plugin (no metadata)",
                         fileName.toStdString());
            continue;
        }

        // Проверка ошибок
        if(!loader.load()) {
            spdlog::error("Failed to load plugin {}: {}",
                          fileName.toStdString(),
                          loader.errorString().toStdString());
            continue;
        }
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
                        assert(!"may be u haven't license!");
                        qDebug( "Plugin Rastr no load (rastr==null)! may be u haven't license!" );
                        continue;
                    }
                    //m_sp_qastra = std::move( std::make_shared<QAstra>());

                    m_sp_qastra = std::make_shared<QAstra>();
                    m_sp_qastra->setRastr(rastr);

                }catch(const std::exception& ex){
                    exclog(ex);
                }catch(...){
                    exclog();
                }
                spdlog::info( "it is Rastr.test.finished");
                qInfo()<< "Plugin Rastr is loaded";
            }
            auto iTI = qobject_cast<InterfaceTI *>(plugin);
            if(iTI){
                try{
                    spdlog::info( "it is TI" );
                    const std::shared_ptr<spdlog::logger> sp_logger = spdlog::default_logger();
                    iTI->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainTI> TI = iTI->getIPlainTIPtr(); // Destroyable  TI{ iTI };
                    if(nullptr==TI){
                        spdlog::error( "TI==null" );
                        continue;
                    }

                    TI->Set_Rastr(m_sp_qastra->getRastr().get());
                    m_sp_qti = std::make_shared<QTI>();
                    m_sp_qti->setTI(TI);

                }catch(const std::exception& ex){
                    exclog(ex);
                }catch(...){
                    exclog();
                }
                spdlog::info( "it is TI.test.finished");
                qInfo()<< "Plugin Ti is loaded";
            }
            auto iBarsMDP = qobject_cast<InterfaceBarsMDP *>(plugin);
            if(iBarsMDP){
                try{
                    spdlog::info( "it is BarsMDP" );
                    const std::shared_ptr<spdlog::logger> sp_logger = spdlog::default_logger();
                    iBarsMDP->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainBarsMDP> BarsMDP = iBarsMDP->getIPlainBarsMDPPtr();
                    if(nullptr==BarsMDP){
                        spdlog::error( "BarsMDP==null" );
                        continue;
                    }

                    BarsMDP->Set_Rastr(m_sp_qastra->getRastr().get());
                    auto ret =BarsMDP->Hello();
                    m_sp_qbarsmdp = std::make_shared<QBarsMDP>();
                    m_sp_qbarsmdp->setBarsMDP(BarsMDP);

                }catch(const std::exception& ex){
                    exclog(ex);
                }catch(...){
                    exclog();
                }
                spdlog::info( "it is BarsMDP.test.finished");
                qInfo()<< "Plugin BarsMDP is loaded";
            }
        }
    }
}

// form files are deployed in form catalog near qrastr.exe
long App::readForms(){
    try{
        //std::vector<std::string> forms = split(str_path_forms, ',');
        std::filesystem::path path_forms ("form");
        std::filesystem::path path_form_load;

        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        //for (std::string &form : forms){
        for(const Params::_v_forms::value_type &form : Params::GetInstance()->getStartLoadForms()){
            std::filesystem::path path_file_form = stringutils::utf8_decode(form);
            path_form_load =  path_forms / path_file_form;
    #if(defined(_MSC_VER))
            qDebug() << "read form from file : " << path_form_load.wstring();
    #else
            //path_forms_load = str_path_to_file_forms;
            //qDebug() << "read form from file : " << path_forms_load.c_str();
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

long App::start(){
    qInfo() << "=== App::start() START ===";
    try{
        long n_res =0;
        QDir::setCurrent(Params::GetInstance()->getDirData().absolutePath());
        loadPlugins();
        if(nullptr!=m_sp_qastra){
            const QDir dir = Params::GetInstance()->getDirSHABLON();
            //std::filesystem::path path_templates = Params::GetInstance()->getDirSHABLON().canonicalPath().toStdString();
#if(QT_VERSION > QT_VERSION_CHECK(5, 16, 0))
            const std::filesystem::path path_templates = Params::GetInstance()->getDirSHABLON().filesystemCanonicalPath();
#else
            std::filesystem::path path_templates = Params::GetInstance()->getDirSHABLON().canonicalPath().toStdString();
            //assert(!"what?");
#endif
            const Params::_v_templates v_templates{ Params::GetInstance()->getStartLoadTemplates() };
            for(const Params::_v_templates::value_type& templ_to_load : v_templates){
                std::filesystem::path path_template = path_templates;
                path_template /= templ_to_load;
                //path_template += "/" + templ_to_load;
                m_sp_qastra->Load( eLoadCode::RG_REPL, "", path_template.string() );
            }
            for(const Params::_v_file_templates::value_type& file_template : Params::GetInstance()->getStartLoadFileTemplates()){
                std::filesystem::path path_template = path_templates;
                path_template /= file_template.second;
                m_sp_qastra->Load( eLoadCode::RG_REPL, file_template.first, path_template.string() );
                if(n_res<0){
                    spdlog::error("{} =LoadFile()", n_res);
                    QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                    //QString("error: %1 wheh read file : %2").arg(n_res).arg(m_params.Get_on_start_load_file_rastr().c_str())
                                    QString("error: %1 wheh read file : %2").arg(n_res).arg(file_template.first.c_str())
                                   );
                    mb.exec();
                }
            }
        }
        spdlog::info("ReadForms");
        n_res = readForms();
        if(n_res<0){
            spdlog::error("{} =ReadForms()", n_res);
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QObject::tr("Can't read forms")
                            //QString("error: %1 wheh read file : %2").arg(n_res).arg(m_params.Get_on_start_load_file_forms().c_str())
                            //QString("error: %1 wheh read file : %2").arg(n_res).arg(Params::GetInstance()->Get_on_start_load_file_forms().c_str())
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
    qInfo() << "=== App::start() END ===";
    return 1;
}

