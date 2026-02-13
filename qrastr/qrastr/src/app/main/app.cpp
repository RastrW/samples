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
#include "plugins/rastr/plugin_interfaces.h"
#include "plugins/ti/plugin_ti_interfaces.h"
#include "plugins/barsmdp/plugin_barsmdp_interfaces.h"
#include "qastra.h"
#include "qti.h"
#include "qbarsmdp.h"
#include "astra_headers/UIForms.h"

App::App(int &argc, char **argv)
    :QApplication(argc, argv){
    upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
}

App::~App() {
    Params::destruct();
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
    }catch (...){
        exclog();
    }
    return done;
}

bool App::init(){
    try{
        auto logg = std::make_shared<spdlog::logger>( "qrastr" );
        spdlog::set_default_logger(logg);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logg->sinks().push_back(console_sink);
        bool res = readSettings();

        const bool bl_res = QDir::setCurrent(Params::get_instance()->getDirData().path());
            assert(bl_res==true);
        fs::path path_log{ Params::get_instance()->getDirData().absolutePath().toStdString() };
        path_log /= L"qrastr_log.txt";
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( path_log.string(), 1024*1024*1, 3);
        std::vector<spdlog::sink_ptr> sinks{ console_sink, rotating_sink };
        logg->sinks().push_back(rotating_sink);

        spdlog::info( "ReadSetting: {}", res);
        spdlog::info( "Log: {}", path_log.generic_u8string() );
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool App::readSettings(){
    try{
        Params::construct();
        QSettings settings(Params::pch_org_qrastr_);
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 800)).toSize();

        int nRes = 0;
        QString qstr_curr_path = QDir::currentPath();
        std::string str_path_2_conf = "undef";
#if(defined(COMPILE_WIN))
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+
                          Params::pch_dir_data_ +"/"+ Params::pch_fname_appsettings;
#else
        //str_path_2_conf = R"(/home/ustas/projects/git_web/samples/qrastr/qrastr/appsettings.json)";
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+Params::pch_dir_data_ +"/"+ Params::pch_fname_appsettings;
       // QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QString("In lin not implemented!") );  mb.exec();
#endif
        QFileInfo fi_appsettings(str_path_2_conf.c_str());
        auto* const p_params = Params::get_instance();
        if(p_params!=nullptr){
            p_params->setDirData(fi_appsettings.dir());
            const bool bl_res = QDir::setCurrent(p_params->getDirData().path());
            if(bl_res == true){
                m_v_cache_log.add(spdlog::level::info, "Set DataDir: {}",
                                  p_params->getDirData().path().toStdString());
            }else{
                m_v_cache_log.add(spdlog::level::err, "Can't set DataDir: {}",
                                  p_params->getDirData().path().toStdString());
            }
            nRes = p_params->readJsonFile(str_path_2_conf);
            if(nRes < 0){
                QMessageBox mb;
                // так лучше не делать ,смешение строк qt и std это боль.
                std::string ss = "error in files load";
                QString str = QString::fromUtf8(ss.c_str());
                mb.setText(str);
                m_v_cache_log.add(spdlog::level::err, "{} ReadJsonFile {}",
                                  nRes, str.toStdString());
                mb.exec();
                return false;
            }
            p_params->setFileAppsettings(str_path_2_conf);
            m_v_cache_log.add(spdlog::level::info, "ReadTemplates: {}",
                              p_params->getDirSHABLON().absolutePath().toStdString());
            nRes = p_params->readTemplates();
            assert(nRes>0);
            if(nRes < 0){
                m_v_cache_log.add(spdlog::level::err, "Error while read: {}", nRes);
            }
            m_v_cache_log.add(spdlog::level::info, "ReadForms: {}",
                              p_params->getPathForms().string());
            nRes = p_params->readFormsExists();
            assert(nRes>0);
            if(nRes < 0){
                m_v_cache_log.add(spdlog::level::err, "Error while read existed forms: {}", nRes);
            }
            nRes = p_params->readForms();
            assert(nRes>0);
            if(nRes < 0){
                m_v_cache_log.add(spdlog::level::err, "Error while read: {}", nRes);
            }

        }else{
            m_v_cache_log.add(spdlog::level::err, "Can't create singleton Params");
        }
    }catch(const std::exception& ex){
        m_v_cache_log.add( spdlog::level::err,"Exception: {} ", ex.what());
        return false;
    }catch(...){
        m_v_cache_log.add( spdlog::level::err,"Unknown exception.");
        return false;
    }
    return true;
}

bool App::loadPlugins(){
    QDir pluginsDir{QDir{QCoreApplication::applicationDirPath()}};
    pluginsDir.cd("plugins");
    spdlog::info("Plugins dir: {}", pluginsDir.absolutePath().toStdString());

    //путь к плагинам в библиотечные пути
    QCoreApplication::addLibraryPath(pluginsDir.absolutePath());

#if(COMPILE_WIN)
    auto entryList = pluginsDir.entryList(QStringList() << "*" + QString(SHLIB_SUFFIX), QDir::Files);
#else
    auto entryList = pluginsDir.entryList(QDir::Files);
#endif

    // move rastr.dll at top
    int ind_rastr = entryList.indexOf(QString(LIB_PREFIX) + "rastr" + QString(SHLIB_SUFFIX));
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
            spdlog::warn("{} is not a valid Qt plugin (no metadata)", fileName.toStdString());
            continue;
        }

        // Проверка ошибок
        if(!loader.load()) {
            spdlog::critical("Failed to load plugin {} : {}", fileName.toStdString(), loader.errorString().toStdString());
            return false;
        }

        QObject *plugin = loader.instance();

        if(plugin){
            spdlog::info( "Load dynamic plugin {}/{} : {}", pluginsDir.absolutePath().toStdString(), fileName.toStdString(), plugin->objectName().toStdString());
            auto iRastr = qobject_cast<InterfaceRastr *>(plugin);
            if(iRastr){
                try{
                    spdlog::info( "it is Rastr" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iRastr->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainRastr> rastr =
                        iRastr->getIPlainRastrPtr(); // Destroyable rastr{ iRastr };
                    if(nullptr==rastr){
                        spdlog::error( "rastr==null" );
                        assert(!"may be u haven't license!");
                        qInfo( "Plugin Rastr no load (rastr==null)! may be u haven't license!" );
                        return false;
                    }

                    m_sp_qastra = std::make_shared<QAstra>();
                    m_sp_qastra->setRastr(rastr);
                }catch(const std::exception& ex){
                    exclog(ex);
                    return false;
                }catch(...){
                    exclog();
                    return false;
                }
                spdlog::info( "it is Rastr.test.finished");
            }
            auto iTI = qobject_cast<InterfaceTI *>(plugin);
            if(iTI){
                try{
                    spdlog::info( "it is TI" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iTI->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainTI> TI = iTI->getIPlainTIPtr(); // Destroyable  TI{ iTI };
                    if(nullptr==TI){
                        spdlog::error( "TI==null" );
                        continue;
                    }

                    auto rastrPtr = m_sp_qastra->getRastr().get();
                    if (rastrPtr == nullptr) {
                        spdlog::critical("Rastr pointer: {}", (void*)rastrPtr);
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
            }
            auto iBarsMDP = qobject_cast<InterfaceBarsMDP *>(plugin);
            if(iBarsMDP){
                try{
                    spdlog::info( "it is BarsMDP" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iBarsMDP->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainBarsMDP> BarsMDP =
                        iBarsMDP->getIPlainBarsMDPPtr();
                    if(BarsMDP == nullptr){
                        spdlog::error( "BarsMDP==null" );
                        continue;
                    }

                    auto rastrPtr = m_sp_qastra->getRastr().get();
                    if (rastrPtr == nullptr) {
                        spdlog::critical("Rastr pointer: {}", (void*)rastrPtr);
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
            }
        }else{
            spdlog::warn("Plugin instance is NULL for {}", fileName.toStdString());
            return false;
        }
    }

    return true;
}

bool App::readForms(){
    try{
        std::filesystem::path path_forms ("form");
        std::filesystem::path path_form_load;

        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        for(const Params::_v_forms::value_type &form : Params::get_instance()->getStartLoadForms()){
            std::filesystem::path path_file_form = stringutils::utf8_decode(form);
            path_form_load =  path_forms / path_file_form;

            auto tempCollection = std::make_unique<CUIFormsCollection>();

            if (path_form_load.extension() == ".fm")
                *tempCollection = CUIFormCollectionSerializerBinary
                                       (path_form_load).Deserialize();
            else
                *tempCollection = CUIFormCollectionSerializerJson
                                       (path_form_load).Deserialize();
            for(const  CUIForm& uiform : tempCollection->Forms()){
                upCUIFormsCollection_->Forms().emplace_back(uiform);
            }
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

std::list<CUIForm>& App::getForms() const {
    assert(nullptr!=upCUIFormsCollection_);
    return upCUIFormsCollection_->Forms();
}

bool App::start(){
    try{
        auto* const p_params = Params::get_instance();
        QDir::setCurrent(p_params->getDirData().absolutePath());
        if (!loadPlugins()){
            return false;
        }
        if(nullptr!=m_sp_qastra){
            const QDir dir = p_params->getDirSHABLON();
#if(QT_VERSION > QT_VERSION_CHECK(5, 16, 0))
            const std::filesystem::path path_templates = p_params->getDirSHABLON().filesystemCanonicalPath();
#else
            std::filesystem::path path_templates = Params::GetInstance()->getDirSHABLON().canonicalPath().toStdString();
            //assert(!"what?");
#endif
            const Params::_v_templates v_templates{ p_params->getStartLoadTemplates() };
            for(const Params::_v_templates::value_type& templ_to_load : v_templates){
                std::filesystem::path path_template = path_templates;
                path_template /= templ_to_load;
                IPlainRastrRetCode res =
                    m_sp_qastra->Load( eLoadCode::RG_REPL, "", path_template.string() );
                if(res != IPlainRastrRetCode::Ok){
                    spdlog::error("wheh read file");
                    QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                   QString("error: wheh read file : %1").arg(templ_to_load.c_str())
                                   );
                    mb.exec();
                }
            }
            for(const Params::_v_file_templates::value_type& file_template
                 : p_params->getStartLoadFileTemplates()){
                std::filesystem::path path_template = path_templates;
                path_template /= file_template.second;
                IPlainRastrRetCode res =
                    m_sp_qastra->Load( eLoadCode::RG_REPL, file_template.first,
                                                           path_template.string() );
                if(res != IPlainRastrRetCode::Ok){
                    spdlog::error("wheh read file");
                    QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                    QString("error: wheh read file : %1").arg(file_template.first.c_str())
                                   );
                    mb.exec();
                }
            }
        }
        spdlog::info("ReadForms");
        if(!readForms()){
            spdlog::error("Can't read forms");
            QMessageBox mb(QMessageBox::Icon::Critical, QObject::tr("Error"),
                           QObject::tr("Can't read forms"));
            mb.exec();
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool App::writeSettings(){
    QSettings settings(Params::pch_org_qrastr_);
    QString qstr = settings.fileName();
    return true;
}
