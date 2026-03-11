#include "graphServer.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QThread>

GraphServer* GraphServer::s_instance = nullptr;

GraphServer::GraphServer(IPlainRastr* rastr, QObject* parent)
    : QObject(parent), m_rastr(rastr)
{
    // Имя библиотеки без расширения — QLibrary сам добавит .so / .dll
    const QString libDir = QCoreApplication::applicationDirPath() + "/plugins/";
#ifdef Q_OS_WIN
    m_lib.setFileName(libDir + "SVGgenerator");
#else
    m_lib.setFileName(libDir + "SVGgenerator");
#endif
}

GraphServer::~GraphServer() {
    stop();
}

bool GraphServer::start() {
    if (m_running.load()) {
        return true; // уже запущен
    }

    m_stopFlag.store(false);
    m_thread = std::thread(&GraphServer::threadFunc, this);
    return true;
}

void GraphServer::stop() {
    if (!m_running.load() && !m_thread.joinable() && !m_lib.isLoaded()) return;

    m_stopFlag.store(true);
    m_cv.notify_all();          // будим поток, если ждёт

    if (m_thread.joinable())
        m_thread.join();        // ждём корректного завершения

    // Выгружаем библиотеку ПОСЛЕ завершения потока
    if (m_lib.isLoaded()) {
        m_lib.unload();
        qInfo() << "GraphServer: library unloaded";
    }

    s_instance = nullptr;

    QMetaObject::invokeMethod(this, [this]() {
        qInfo() << "[sig_stopped] delivered in thread:" << QThread::currentThreadId();
        emit sig_stopped();
    }, Qt::QueuedConnection);
}

void GraphServer::threadFunc() {
    m_running.store(true);

    // --- Загрузка DLL ---
    if (!m_lib.load()) {
        qCritical() << "GraphServer: failed to load library:"
                    << m_lib.errorString();
        return;
    }
    qInfo() << "GraphServer: library loaded:" << m_lib.fileName();

    loadSymbols();

    if (!m_fnInit || !m_fnPutTextLayer || !m_fnUpdateAll) {
        qWarning() << "GraphServer: required symbols not found";
        m_running.store(false);
        return;
    }

    // --- Пути ---
    QString graphLibsPath = QCoreApplication::applicationDirPath()
                            + "/plugins/graph2libs.xml";
    if (!QFile::exists(graphLibsPath)) {
        qWarning() << "GraphServer: graph2libs.xml not found";
        m_running.store(false);
        return;
    }

    // --- Запуск HTTP-сервера через dll ---
    s_instance = this;
    m_fnInit(m_rastr,
             graphLibsPath.toStdString().c_str(),
             "127.0.0.1", 8081,
             &GraphServer::staticCallback);

    qInfo() << "InitPlainDLL_t is ready";
    // можно открывать QWebEngineView
    QMetaObject::invokeMethod(this, [this]() {
        qInfo() << "[sig_ready] delivered in thread:" << QThread::currentThreadId();
        emit sig_ready();
    }, Qt::QueuedConnection);

    // --- Цикл обработки колбэков ---
    while (!m_stopFlag.load()) {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Ждём событие ИЛИ флаг остановки
        m_cv.wait(lock, [this] {
            return !m_calls.empty() || m_stopFlag.load();
        });

        while (!m_calls.empty()) {
            auto [iMSG, params] = *m_calls.begin();
            m_calls.erase(m_calls.begin());
            lock.unlock();
            dispatchCallback(iMSG, params);
            lock.lock();
        }
    }

    m_running.store(false);
}

void GraphServer::dispatchCallback(int iMSG, const std::string& params) {
    enum class CB : int {
        LayerLoadStart   = 1400,
        FullRefresh      = 1799,
        ReverseState     = 1800,
        DeleteNode       = 1801,
        MoveNode         = 1802,
        AddNode          = 1803
    };

    switch (iMSG) {
    case (int)CB::FullRefresh:
        if (m_fnUpdateAll) m_fnUpdateAll();
        break;
    case (int)CB::DeleteNode:
            m_fnRemoveNode(std::stoi(params));
        break;
    case (int)CB::MoveNode:
    case (int)CB::AddNode:
    {
        int start(0), end(0);
        std::vector<std::string> v;
        while ((start = params.find_first_not_of('&', end))!= std::string::npos)
        {
            end = params.find('&', start);
            v.push_back(params.substr(start, end - start));
        }
        if(v.size() == 3)
            m_fnMoveOrAdd(std::stoi(v[0]),std::stoi(v[1]), std::stoi(v[2]));
    }
        break;
    case 1400: case 1401: case 1402: case 1403: case 1404:
        if (m_fnPutTextLayer) m_fnPutTextLayer(iMSG - 1400);
        break;
    default:
        break;
    }
}

void GraphServer::loadSymbols() {
    auto resolve = [&](const char* name) -> QFunctionPointer {
        auto fn = m_lib.resolve(name);
        if (!fn)
            qWarning() << "GraphServer: symbol not found:" << name;
        return fn;
    };

    m_fnInit         = reinterpret_cast<InitPlainDLL_t>      (resolve("InitPlainDLL"));
    m_fnPutTextLayer = reinterpret_cast<PutTextLayer_t>      (resolve("PutTextLayer"));
    m_fnUpdateAll    = reinterpret_cast<UpdateAllContent_t>  (resolve("UpdateAllContent"));
    m_fnRemoveNode   = reinterpret_cast<RemoveGraphNode_t>   (resolve("RemoveGraphNode"));
    m_fnMoveOrAdd    = reinterpret_cast<MoveOrAddGraphNode_t>(resolve("MoveOrAddGraphNode"));
}

/*static*/ void GraphServer::staticCallback(int iMSG, const char* params) {
    if (!s_instance) return;
    std::scoped_lock lock(s_instance->m_mutex);
    s_instance->m_calls.emplace(iMSG, params ? params : "");
    s_instance->m_cv.notify_one();
}
