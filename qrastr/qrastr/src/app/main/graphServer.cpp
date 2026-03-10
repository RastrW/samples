#include "graphServer.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

#include <dlfcn.h>

GraphServer* GraphServer::s_instance = nullptr;

GraphServer::GraphServer(IPlainRastr* rastr, QObject* parent)
    : QObject(parent), m_rastr(rastr)
{}

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
    if (!m_running.load() && !m_thread.joinable()) return;

    m_stopFlag.store(true);
    m_cv.notify_all();          // будим поток, если ждёт

    if (m_thread.joinable())
        m_thread.join();        // ждём корректного завершения

    if (m_dllHandle) {
        dlclose(m_dllHandle);
        m_dllHandle = nullptr;
    }
    s_instance = nullptr;
    emit sig_stopped();
}

void GraphServer::threadFunc() {
    m_running.store(true);

    // --- Загрузка dll ---
    QString libPath = QCoreApplication::applicationDirPath()
                      + "/plugins/libSVGgenerator.so";

    m_dllHandle = dlopen(libPath.toStdString().c_str(), RTLD_NOW);
    if (!m_dllHandle) {
        qWarning() << "GraphServer: dlopen failed:" << dlerror();
        m_running.store(false);
        return;
    }

    qWarning() << "GraphServer: dlopen succesfull";

    m_fnInit       = reinterpret_cast<InitPlainDLL_t>    (dlsym(m_dllHandle, "InitPlainDLL"));
    m_fnPutTextLayer = reinterpret_cast<PutTextLayer_t>  (dlsym(m_dllHandle, "PutTextLayer"));
    m_fnUpdateAll  = reinterpret_cast<UpdateAllContent_t>(dlsym(m_dllHandle, "UpdateAllContent"));
    m_fnRemoveNode = reinterpret_cast<RemoveGraphNode_t> (dlsym(m_dllHandle, "RemoveGraphNode"));
    m_fnMoveOrAdd  = reinterpret_cast<MoveOrAddGraphNode_t>(dlsym(m_dllHandle, "MoveOrAddGraphNode"));

    if (!m_fnInit || !m_fnPutTextLayer || !m_fnUpdateAll) {
        qWarning() << "GraphServer: required symbols not found";
        dlclose(m_dllHandle);
        m_dllHandle = nullptr;
        m_running.store(false);
        return;
    }

    // --- Пути ---
    QString graphLibsPath = QCoreApplication::applicationDirPath()
                            + "/plugins/graph2libs.xml";
    if (!QFile::exists(graphLibsPath)) {
        qWarning() << "GraphServer: graph2libs.xml not found";
        dlclose(m_dllHandle);
        m_dllHandle = nullptr;
        m_running.store(false);
        return;
    }

    // --- Запуск HTTP-сервера через dll ---
    s_instance = this;
    m_fnInit(m_rastr,
             graphLibsPath.toStdString().c_str(),
             "127.0.0.0", 8081,
             &GraphServer::staticCallback);

    qInfo() << "InitPlainDLL_t is ready";
    emit sig_ready();   // можно открывать QWebEngineView

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
        // TODO
        break;
    case (int)CB::MoveNode:
    case (int)CB::AddNode:
        // TODO:
        break;
    case 1400: case 1401: case 1402: case 1403: case 1404:
        if (m_fnPutTextLayer) m_fnPutTextLayer(iMSG - 1400);
        break;
    default:
        break;
    }
}

/*static*/ void GraphServer::staticCallback(int iMSG, const char* params) {
    if (!s_instance) return;
    std::scoped_lock lock(s_instance->m_mutex);
    s_instance->m_calls.emplace(iMSG, params ? params : "");
    s_instance->m_cv.notify_one();
}
