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
        logg->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logg->sinks().push_back(console_sink);

        bool res = readSettings();

        const bool bl_res = QDir::setCurrent(Params::get_instance()->getDirData().path());
            assert(bl_res==true);

        fs::path path_log{ Params::get_instance()->getDirData().absolutePath().toStdString() };
        path_log /= L"qrastr_log.txt";

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
            ( path_log.string(), 1024*1024*1, 3);
        logg->sinks().push_back(file_sink);

        // Теперь есть консоль + файл — сбрасываем кэш readSettings
        m_v_cache_log.add(spdlog::level::info, "ReadSetting: {}", res);
        m_v_cache_log.add(spdlog::level::info, "Log: {}", path_log.generic_u8string());
        m_v_cache_log.flush(); // ранние сообщения уйдут в консоль и файл
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

// ---------------------------- Graph ----------------------------
std::mutex ws_mutex;
std::condition_variable ws_cv;
std::map<int, std::string> calls;

void AsyncCallback2(int iMSG, const char *params)
{
    std::scoped_lock<std::mutex> lock(ws_mutex);
    calls.insert(std::make_pair(iMSG, params));
    ws_cv.notify_one();			//пнуть главный поток для обработки асинхронного сообщения

    qDebug() << "AsyncCallback2 " << iMSG << endl;

};

void task_run_graph( IPlainRastr* sp_rastr) {
    //sp_astra->runGraph();

    // Графика
    enum class _callbackGraph : int
    {
        LayerLoadStart = 1400,
        FullRefreshControl = 1799,
        ReverseState = 1800,
        DeleteNode = 1801,
        MoveNode = 1802,
        AddNode = 1803
    };
    using AsyncCallback = void(int iMSG, const char *params);
    //extern "C" SBtype void InitPlainDLL(IPlainRastr* pPLain, const char *libpath, const char* ip, long port, AsyncCallback Hh)
    using InitPlainDLL_t = void(*)(IPlainRastr* pPLain, const char* libpath, const char* ip, long port, AsyncCallback fn);
    using PutTextLayer_t = void(*)(long iLayer);
    using UpdateAllContent_t = void(*)();
    using RemoveGraphNode_t = void(*)(long inode);
    using MoveOrAddGraphNode_t = void(*)(long inode,long x,long y);

    const char* pch_name_init_plain_dll {"InitPlainDLL"};
    const char* pch_name_put_text_llayer {"PutTextLayer"};
    const char* pch_name_update_all_content{"UpdateAllContent"};
    const char* pch_name_remoove_graphnode {"RemoveGraphNode"};
    const char* pch_name_move_or_add_graphnode {"MoveOrAddGraphNode"};

    QString qstr_path_graph{QCoreApplication::applicationDirPath()};
    qstr_path_graph += "/plugins/libSVGgenerator.so";
    void *phsvg = dlopen(qstr_path_graph.toStdString().c_str(), RTLD_NOW);
    if(phsvg != nullptr)
    {
        InitPlainDLL_t pInitPlainDLL = reinterpret_cast<InitPlainDLL_t>(dlsym(phsvg, pch_name_init_plain_dll));
        PutTextLayer_t pPutTextLayer = reinterpret_cast<PutTextLayer_t>(dlsym(phsvg, pch_name_put_text_llayer));
        UpdateAllContent_t pUpdateAllContent = reinterpret_cast<UpdateAllContent_t>(dlsym(phsvg, pch_name_update_all_content));
        RemoveGraphNode_t pRemoveGraphNode = reinterpret_cast<RemoveGraphNode_t>(dlsym(phsvg, pch_name_remoove_graphnode));
        MoveOrAddGraphNode_t pMoveOrAddGraphNode = reinterpret_cast<MoveOrAddGraphNode_t>(dlsym(phsvg, pch_name_move_or_add_graphnode));
        if(pInitPlainDLL && pPutTextLayer && pUpdateAllContent)
        {
            QString qstr_test_file_path{QCoreApplication::applicationDirPath()};
            QString qstr_graph2libs_path{QCoreApplication::applicationDirPath()};
            qstr_graph2libs_path.append("/plugins/graph2libs.xml");
            qstr_test_file_path.append("/../Data/Files/all");

            if (sp_rastr != nullptr && QFile::exists(qstr_test_file_path) && QFile::exists(qstr_graph2libs_path))
            {
                //sp_rastr->Load(eLoadCode::RG_REPL,qstr_test_file_path.toStdString().c_str(),"");
                bool run = true;
                if (run)
                {
                    pInitPlainDLL(sp_rastr, qstr_graph2libs_path.toStdString().c_str(), "127.0.0.0", 8081, AsyncCallback2);

                    for(;;)
                    {
                        std::unique_lock<std::mutex> lock(ws_mutex);
                        while(!calls.empty())
                        {
                            auto imsg = *calls.begin();
                            calls.erase(calls.begin());
                            switch (imsg.first)
                            {
                                case (int)_callbackGraph::FullRefreshControl:
                                {
                                    pUpdateAllContent();
                                    break;
                                }
                                case (int)_callbackGraph::ReverseState:
                                {
                                    std::string params = imsg.second;
                                    auto it1 = params.find(";");
                                    if(it1 != std::string::npos)
                                    {
                                        std::string tabl = params.substr(0, it1);
                                    }

                                    break;
                                }
                                case (int)_callbackGraph::DeleteNode:
                                {
                                    break;
                                }
                                case (int)_callbackGraph::MoveNode:
                                {
                                    //MoveOrAddGraphNode
                                    break;
                                }
                                case (int)_callbackGraph::AddNode:
                                {
                                    //MoveOrAddGraphNode
                                    break;
                                }
                                case 1400:
                                case 1401:
                                case 1402:
                                case 1403:
                                case 1404:
                                {
                                    pPutTextLayer(imsg.first - 1400);
                                    break;
                                }

                            default:
                                break;
                            }
                        }
                        ws_cv.wait(lock);
                    }
                }
            }
        }
    }
}
// ----------------------------/ Graph ----------------------------

bool App::loadPlugins(){
    QDir pluginsDir{QDir{QCoreApplication::applicationDirPath()}};
    pluginsDir.cd("plugins");
    m_v_cache_log.add(spdlog::level::info, "Plugins dir: {}",
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
            m_v_cache_log.add(spdlog::level::warn,
                              "{} is not a valid Qt plugin (no metadata)", fileName.toStdString());
            continue;
        }

        // Проверка ошибок
        if(!loader.load()) {
            m_v_cache_log.add(spdlog::level::critical,"Failed to load plugin {} : {}",
                              fileName.toStdString(), loader.errorString().toStdString());
            return false;
        }

        QObject *plugin = loader.instance();

        if(plugin){
            m_v_cache_log.add(spdlog::level::info, "Load dynamic plugin {}/{} : {}",
                              pluginsDir.absolutePath().toStdString(), fileName.toStdString(),
                              plugin->objectName().toStdString());
            auto iRastr = qobject_cast<InterfaceRastr *>(plugin);
            if(iRastr){
                try{
                    m_v_cache_log.add(spdlog::level::info, "it is Rastr");
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iRastr->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainRastr> rastr =
                        iRastr->getIPlainRastrPtr(); // Destroyable rastr{ iRastr };
                    if(nullptr==rastr){
                        m_v_cache_log.add(spdlog::level::err, "rastr==null" );
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
                m_v_cache_log.add(spdlog::level::info, "it is Rastr.test.finished");
            }
            auto iTI = qobject_cast<InterfaceTI *>(plugin);
            if(iTI){
                try{
                    m_v_cache_log.add(spdlog::level::info, "it is TI" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iTI->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainTI> TI = iTI->getIPlainTIPtr(); // Destroyable  TI{ iTI };
                    if(nullptr==TI){
                        m_v_cache_log.add(spdlog::level::err, "TI==null" );
                        continue;
                    }

                    auto rastrPtr = m_sp_qastra->getRastr().get();
                    if (rastrPtr == nullptr) {
                        m_v_cache_log.add(spdlog::level::critical,
                                          "Rastr pointer: {}", (void*)rastrPtr);
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
                m_v_cache_log.add(spdlog::level::info, "it is TI.test.finished");
            }
            auto iBarsMDP = qobject_cast<InterfaceBarsMDP *>(plugin);
            if(iBarsMDP){
                try{
                    m_v_cache_log.add(spdlog::level::info, "it is BarsMDP" );
                    const std::shared_ptr<spdlog::logger> sp_logger =
                        spdlog::default_logger();
                    iBarsMDP->setLoggerPtr( sp_logger );
                    const std::shared_ptr<IPlainBarsMDP> BarsMDP =
                        iBarsMDP->getIPlainBarsMDPPtr();
                    if(BarsMDP == nullptr){
                        m_v_cache_log.add(spdlog::level::err, "BarsMDP==null" );
                        continue;
                    }

                    auto rastrPtr = m_sp_qastra->getRastr().get();
                    if (rastrPtr == nullptr) {
                        m_v_cache_log.add(spdlog::level::critical,
                                          "Rastr pointer: {}", (void*)rastrPtr);
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
                m_v_cache_log.add(spdlog::level::info, "it is BarsMDP.test.finished");
            }
        }else{
            m_v_cache_log.add(spdlog::level::warn,
                              "Plugin instance is NULL for {}", fileName.toStdString());
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

bool App::loadGraphlibSVGgenerator(){
    try{
        std::thread thread_graph( task_run_graph, m_sp_qastra->getRastr().get());
        thread_graph.detach();
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
            std::filesystem::path path_templates = p_params->getDirSHABLON().canonicalPath().toStdString();
#endif
            const Params::_v_templates v_templates{ p_params->getStartLoadTemplates() };
            for(const Params::_v_templates::value_type& templ_to_load : v_templates){
                std::filesystem::path path_template = path_templates;
                path_template /= templ_to_load;
                IPlainRastrRetCode res =
                    m_sp_qastra->Load( eLoadCode::RG_REPL, "", path_template.string() );
                if(res != IPlainRastrRetCode::Ok){
                    m_v_cache_log.add(spdlog::level::err,
                                      "Failed to load template: {}", templ_to_load);
                    QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                   QString("error: wheh read file : %1").arg(templ_to_load.c_str())
                                   );
                    mb.exec();
                }else {
                    m_v_cache_log.add(spdlog::level::info,
                                      "Template loaded: {}", templ_to_load);
                }
            }
            for (const auto& file_template : p_params->getStartLoadFileTemplates()) {
                fs::path path_file = file_template.first;

                // Проверяем существование до загрузки
                if (!fs::exists(path_file)) {
                    Params::_v_file_templates empty;
                    p_params->setStartLoadFileTemplates(empty);
                    m_v_cache_log.add(spdlog::level::warn,
                                      "Startup file not found: {}", path_file.string());
                    QMessageBox mb(QMessageBox::Icon::Warning,
                                   QObject::tr("Файл не найден"),
                                   QString("Файл не найден:\n%1\n\nБудет открыт пустой проект.")
                                       .arg(QString::fromStdString(path_file.string())));
                    mb.exec();
                    continue; // Не падаем — просто пропускаем
                }
                try {
                    fs::path path_template = path_templates / file_template.second;
                    IPlainRastrRetCode res = m_sp_qastra->Load(
                        eLoadCode::RG_REPL,
                        file_template.first,
                        path_template.string()
                        );
                    if (res != IPlainRastrRetCode::Ok) {
                        Params::_v_file_templates empty;
                        p_params->setStartLoadFileTemplates(empty);
                        m_v_cache_log.add(spdlog::level::err,
                                          "Failed to load file template: {}", file_template.first);
                    }
                } catch (const std::exception& ex) {
                    Params::_v_file_templates empty;
                    p_params->setStartLoadFileTemplates(empty);
                    m_v_cache_log.add(spdlog::level::err,
                                      "Exception loading startup file {}: {}",
                                      file_template.first, ex.what());
                    QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                   QString("error: wheh read file : %1").arg(file_template.first.c_str())
                                   );
                    mb.exec();
                }
            }
        }
        m_v_cache_log.add(spdlog::level::info, "ReadForms: starting");
        if(!readForms()){
            m_v_cache_log.add(spdlog::level::err, "Can't read forms");
            QMessageBox mb(QMessageBox::Icon::Critical, QObject::tr("Error"),
                           QObject::tr("Can't read forms"));
            mb.exec();
        }else {
            m_v_cache_log.add(spdlog::level::info, "ReadForms: OK");
        }

        m_v_cache_log.add(spdlog::level::info, "ReadForms: starting");
        if (!loadGraphlibSVGgenerator())
        {

        }else {
            m_v_cache_log.add(spdlog::level::info, "loadGraphlibSVGgenerator: OK");
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

void App::flushLogCache() {
    m_v_cache_log.flush();
}
