#include "app.h"

#include <memory>
#include <QString>
#include <QMessageBox>
#include <QPluginLoader>
#include <QPluginLoader>
#include "common_qrastr.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/qt_sinks.h>
#include "rastrParameters.h"
using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>
#include "plugins/rastr/plugin_interfaces.h"
#include "plugins/ti/plugin_ti_interfaces.h"
#include "plugins/barsmdp/plugin_barsmdp_interfaces.h"
#include "qastra.h"
#include "qti.h"
#include "qbarsmdp.h"
#include "astra_headers/UIForms.h"
#include "startupLoader.h"

class QtSink : public spdlog::sinks::base_sink<std::mutex>
{
protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        QString s = QString::fromUtf8(msg.payload.data(), static_cast<int>(msg.payload.size()));
        qInfo() << s;
    }
    void flush_() override {}
};

App::App(int &argc, char **argv)
    :QApplication(argc, argv){
    upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
}

App::~App() {
    //останавливаем фоновые потоки и гарантирует финальный сброс:
    spdlog::shutdown();
    RastrParameters::destruct();
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
        logg->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

#ifdef _DEBUG
        ///@todo не удается решить проблему с кодировкой вывода spdlog в коносоль
        auto qt_sink = std::make_shared<QtSink>();
        logg->sinks().push_back(qt_sink);
#else
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logg->sinks().push_back(console_sink);
#endif
        bool res = readSettings();

        const bool bl_res = QDir::setCurrent(RastrParameters::get_instance()->getDirData().path());
            assert(bl_res==true);

        fs::path path_log{ RastrParameters::get_instance()->getDirData().absolutePath().toStdString() };
        path_log /= L"qrastr_log.txt";

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
            ( path_log.string(), 1024*1024*1, 3);
        logg->sinks().push_back(file_sink);
        spdlog::flush_every(std::chrono::seconds(1));

        // Теперь есть консоль + файл — сбрасываем кэш readSettings
        m_v_cache_log.add(spdlog::level::info, "ReadSetting: {}", res);
        m_v_cache_log.add(spdlog::level::info, "Log: {}", path_log.generic_u8string());
        // Флашим ТОЛЬКО в file+console синки, НЕ очищаем кэш:
#ifdef _DEBUG
        m_v_cache_log.flushToSinks({qt_sink, file_sink});
#else
        m_v_cache_log.flushToSinks({console_sink, file_sink});
#endif
        //m_v_cache_log` нужен **только** для сообщений, которые появляются **до** инициализации синков
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
        RastrParameters::construct();
        QString qstr_curr_path = QDir::currentPath();
        std::string str_path_2_conf = "undef";
#if(defined(COMPILE_WIN))
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+
                          RastrParameters::pch_dir_data_ +"/"+ RastrParameters::pch_fname_appsettings;
#else
        //str_path_2_conf = R"(/home/ustas/projects/git_web/samples/qrastr/qrastr/appsettings.json)";
        str_path_2_conf = qstr_curr_path.toStdString()+ "/../"+Params::pch_dir_data_ +"/"+ Params::pch_fname_appsettings;
        // QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QString("In lin not implemented!") );  mb.exec();
#endif
        QFileInfo fi_appsettings(str_path_2_conf.c_str());
        auto* const p_params = RastrParameters::get_instance();
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
            if(!p_params->readJsonFile(str_path_2_conf)){
                //повреждённый JSON или проблемы с правами доступа
                m_v_cache_log.add(spdlog::level::err,
                                  "ReadJsonFile failed: {}", str_path_2_conf);
                QMessageBox mb(QMessageBox::Icon::Critical,
                               QObject::tr("Error"),
                               QObject::tr("Failed to parse appsettings.json.\n"
                                           "Check file for errors or delete it to reset defaults."));
                mb.exec();
                return false;
            }
            p_params->setFileAppsettings(str_path_2_conf);
            m_v_cache_log.add(spdlog::level::info, "ReadTemplates: {}",
                              p_params->getDirSHABLON().absolutePath().toStdString());

            if(!p_params->readTemplates()){
                m_v_cache_log.add(spdlog::level::err, "Error while read");
            }
            m_v_cache_log.add(spdlog::level::info, "ReadForms: {}",
                              p_params->getPathForms().string());
            if(!p_params->readFormsExists()){
                m_v_cache_log.add(spdlog::level::err, "Error while read existed forms");
            }
            if(!p_params->readForms()){
                m_v_cache_log.add(spdlog::level::err, "Error while read");
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

bool App::start() {
    try {
        auto* params = RastrParameters::get_instance();
        QDir::setCurrent(params->getDirData().absolutePath());

        emit sig_progressChanged(35, tr("Загрузка плагинов..."));
        if (!loadPlugins())
            return false;

        // Загрузка стартовых шаблонов и файлов.
        emit sig_progressChanged(65, tr("Загрузка шаблонов..."));
        StartupLoader loader(m_sp_qastra);
        connect(&loader, &StartupLoader::loadWarning, [](const QString& msg) {
            spdlog::warn("{}", msg.toStdString());
        });

        if (!loader.load())
            return false;

        spdlog::info("ReadForms: starting");
        emit sig_progressChanged(80, tr("Чтение форм..."));
        if (!deserializeForms()) {
            spdlog::error("Can't read forms");
            QMessageBox::critical(nullptr, tr("Ошибка"), tr("Can't read forms"));
            return false;
        }
    } catch (const std::exception& ex) {
        exclog(ex);
        return false;
    } catch (...) {
        exclog();
        return false;
    }
    return true;
}

bool App::loadPlugins(){
    QDir pluginsDir{QDir{QCoreApplication::applicationDirPath()}};
    pluginsDir.cd("plugins");
    spdlog::info("Plugins dir: {}",
                      pluginsDir.absolutePath().toStdString());

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
            spdlog::critical("Failed to load plugin {} : {}",
                              fileName.toStdString(), loader.errorString().toStdString());
            return false;
        }

        QObject *plugin = loader.instance();
        if(plugin){
           spdlog::info("Load dynamic plugin {}/{} : {}",
                              pluginsDir.absolutePath().toStdString(), fileName.toStdString(),
                              plugin->objectName().toStdString());
            auto iRastr = qobject_cast<InterfaceRastr *>(plugin);
            if(iRastr){
                try{
                    spdlog::info("it is Rastr");
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iRastr->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainRastr> rastr =
                        iRastr->getIPlainRastrPtr(); // Destroyable rastr{ iRastr };
                    if(nullptr==rastr){
                        assert(!"may be u haven't license!");
                        spdlog::critical("Plugin Rastr no load (rastr==null)! may be u haven't license!" );
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
                spdlog::info("it is Rastr.test.finished");
            }
            auto iTI = qobject_cast<InterfaceTI *>(plugin);
            if(iTI){
                try{
                    spdlog::info("it is TI" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iTI->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainTI> TI = iTI->getIPlainTIPtr(); // Destroyable  TI{ iTI };
                    if(nullptr==TI){
                        spdlog::error("TI==null" );
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
                spdlog::info("it is TI.test.finished");
            }
            auto iBarsMDP = qobject_cast<InterfaceBarsMDP *>(plugin);
            if(iBarsMDP){
                try{
                    spdlog::info("it is BarsMDP" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iBarsMDP->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainBarsMDP> BarsMDP =
                        iBarsMDP->getIPlainBarsMDPPtr();
                    if(BarsMDP == nullptr){
                        spdlog::info("BarsMDP==null" );
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
                spdlog::info("it is BarsMDP.test.finished");
            }
        }else{
            spdlog::warn("Plugin instance is NULL for {}", fileName.toStdString());
            return false;
        }
    }

    return true;
}

bool App::deserializeForms(){
    try{
        std::filesystem::path path_forms ("form");
        std::filesystem::path path_form_load;

        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        for(const RastrParameters::_v_forms::value_type &form : RastrParameters::get_instance()->getStartLoadForms()){
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

void App::flushLogCache(std::shared_ptr<spdlog::sinks::sink> qt_sink) {
    m_v_cache_log.flushToSinks({qt_sink}); // только Qt, без дублей
    m_v_cache_log.clear();
}
