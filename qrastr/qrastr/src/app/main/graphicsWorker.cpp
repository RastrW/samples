#include "graphicsWorker.h"

#include <dlfcn.h>

#include <stdexcept>
#include <iostream>
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "CommonConst.h"

// Статические члены
std::mutex                   GraphicsWorker::s_mutex;
std::condition_variable      GraphicsWorker::s_cv;
std::map<int, std::string>   GraphicsWorker::s_pendingCalls;
GraphicsWorker*              GraphicsWorker::s_instance = nullptr;

GraphicsWorker::GraphicsWorker(
    const std::filesystem::path& projectPath,
    const std::filesystem::path& astraLibPath,
    const QString& ip,
    long           port,
    QObject*       parent)
    : QThread(parent)
    , m_projectPath (projectPath)
    , m_astraLibPath(astraLibPath)
    , m_ip  (ip)
    , m_port(port)
{}

GraphicsWorker::~GraphicsWorker()
{
    requestStop();
    // Будим цикл ожидания, чтобы поток вышел из wait()
    {
        std::scoped_lock lock(s_mutex);
        s_cv.notify_one();
    }
    wait(); // Ждём завершения QThread
    unloadLibraries();
}

void GraphicsWorker::requestStop()
{
    m_stopRequested.storeRelaxed(1);
    std::scoped_lock lock(s_mutex);
    s_cv.notify_one();
}

QString GraphicsWorker::serverUrl() const
{
    return QStringLiteral("http://%1:%2/grf.html")
        .arg(m_ip)
        .arg(m_port);
}

void GraphicsWorker::staticAsyncCallback(int iMSG, const char* params)
{
    {
        std::scoped_lock lock(s_mutex);
        s_pendingCalls.emplace(iMSG, params ? params : "");
        s_cv.notify_one();
    }
    // Дублируем в stdout для отладки (как было в оригинале)
    std::cout << "[GraphicsWorker] AsyncCallback iMSG=" << iMSG << '\n';
}

bool GraphicsWorker::loadLibraries()
{
    namespace fs = std::filesystem;

    // 1. libastra.so ──────────────────────────────────────────────────────────
    m_hAstra = dlopen(m_astraLibPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!m_hAstra) {
        emit errorOccurred(
            QStringLiteral("dlopen libastra failed: %1").arg(dlerror()));
        return false;
    }
    emit statusMessage(
        QStringLiteral("libastra loaded: %1")
            .arg(QString::fromStdString(
                fs::weakly_canonical(m_astraLibPath).string())));

    // 2. libSVGgenerator.so ───────────────────────────────────────────────────
    fs::path svgLibPath = m_projectPath / "libSVGgenerator.so";
    m_hSVG = dlopen(svgLibPath.c_str(), RTLD_NOW);
    if (!m_hSVG) {
        emit errorOccurred(
            QStringLiteral("dlopen libSVGgenerator failed: %1").arg(dlerror()));
        return false;
    }
    emit statusMessage(
        QStringLiteral("libSVGgenerator loaded: %1")
            .arg(QString::fromStdString(
                fs::weakly_canonical(svgLibPath).string())));

    // 3. Символы из libastra.so ───────────────────────────────────────────────
    m_fnPlainFactory = reinterpret_cast<rastr_t>(
        dlsym(m_hAstra, "PlainRastrFactory"));
    if (!m_fnPlainFactory) {
        emit errorOccurred(
            QStringLiteral("dlsym PlainRastrFactory: %1").arg(dlerror()));
        return false;
    }

    // 4. Символы из libSVGgenerator.so ────────────────────────────────────────
    auto loadSym = [&](const char* name) -> void* {
        void* sym = dlsym(m_hSVG, name);
        if (!sym) {
            emit errorOccurred(
                QStringLiteral("dlsym %1: %2")
                    .arg(name)
                    .arg(dlerror()));
        }
        return sym;
    };

    m_fnInitPlainDLL    = reinterpret_cast<InitPlainDLL_t>    (loadSym("InitPlainDLL"));
    m_fnPutTextLayer    = reinterpret_cast<PutTextLayer_t>    (loadSym("PutTextLayer"));
    m_fnUpdateAllContent= reinterpret_cast<UpdateAllContent_t>(loadSym("UpdateAllContent"));
    m_fnRemoveGraphNode = reinterpret_cast<RemoveGraphNode_t> (loadSym("RemoveGraphNode"));
    m_fnMoveOrAddNode   = reinterpret_cast<MoveOrAddGraphNode_t>(loadSym("MoveOrAddGraphNode"));

    // RemoveGraphNode и MoveOrAddGraphNode некритичны — не прерываем загрузку.
    if (!m_fnInitPlainDLL || !m_fnPutTextLayer || !m_fnUpdateAllContent) {
        return false; // errorOccurred уже был emit-нут в loadSym
    }

    return true;
}

void GraphicsWorker::unloadLibraries()
{
    if (m_hSVG)   { dlclose(m_hSVG);   m_hSVG   = nullptr; }
    if (m_hAstra) { dlclose(m_hAstra); m_hAstra = nullptr; }
}

void GraphicsWorker::dispatchMessage(int iMSG, const std::string& params)
{
    switch (static_cast<_callbackGraph>(iMSG))
    {
        // Полное обновление графики
        case _callbackGraph::FullRefreshControl:
            if (m_fnUpdateAllContent) m_fnUpdateAllContent();
            break;

        // Смена состояния элемента
        case _callbackGraph::ReverseState: {
            auto sep = params.find(';');
            if (sep != std::string::npos) {
                //[[maybe_unused]] std::string tabl = params.substr(0, sep);
                // TODO: применить изменение через IPlainRastr
            }
            break;
        }

        // Удаление узла
        case _callbackGraph::DeleteNode: {
            // params содержит идентификатор узла
            try {
                long inode = std::stol(params);
                if (m_fnRemoveGraphNode) m_fnRemoveGraphNode(inode);
            } catch (const std::exception& e) {
                emit statusMessage(
                    QStringLiteral("DeleteNode: bad params '%1': %2")
                        .arg(QString::fromStdString(params))
                        .arg(e.what()));
            }
            break;
        }

        // Перемещение узла
        case _callbackGraph::MoveNode: {
            // Ожидаемый формат params: "inode;x;y"
            try {
                auto p1 = params.find(';');
                auto p2 = params.find(';', p1 + 1);
                if (p1 != std::string::npos && p2 != std::string::npos) {
                    long inode = std::stol(params.substr(0, p1));
                    long x     = std::stol(params.substr(p1 + 1, p2 - p1 - 1));
                    long y     = std::stol(params.substr(p2 + 1));
                    if (m_fnMoveOrAddNode) m_fnMoveOrAddNode(inode, x, y);
                }
            } catch (const std::exception& e) {
                emit statusMessage(
                    QStringLiteral("MoveNode: bad params '%1': %2")
                        .arg(QString::fromStdString(params))
                        .arg(e.what()));
            }
            break;
        }

        // Добавление узла
        case _callbackGraph::AddNode: {
            // Ожидаемый формат params: "inode;x;y"
            try {
                auto p1 = params.find(';');
                auto p2 = params.find(';', p1 + 1);
                if (p1 != std::string::npos && p2 != std::string::npos) {
                    long inode = std::stol(params.substr(0, p1));
                    long x     = std::stol(params.substr(p1 + 1, p2 - p1 - 1));
                    long y     = std::stol(params.substr(p2 + 1));
                    if (m_fnMoveOrAddNode) m_fnMoveOrAddNode(inode, x, y);
                }
            } catch (const std::exception& e) {
                emit statusMessage(
                    QStringLiteral("AddNode: bad params '%1': %2")
                        .arg(QString::fromStdString(params))
                        .arg(e.what()));
            }
            break;
        }

        // Загрузка текстовых слоёв (LayerLoadStart + 0..4)
        default: {
            constexpr int kLayerBase  = static_cast<int>(_callbackGraph::LayerLoadStart);
            constexpr int kLayerCount = 5;
            if (iMSG >= kLayerBase && iMSG < kLayerBase + kLayerCount) {
                if (m_fnPutTextLayer) m_fnPutTextLayer(iMSG - kLayerBase);
            } else {
                emit statusMessage(
                    QStringLiteral("Unknown iMSG=%1 params='%2'")
                        .arg(iMSG)
                        .arg(QString::fromStdString(params)));
            }
            break;
        }
    }
}

void GraphicsWorker::run()
{
    namespace fs = std::filesystem;

    // Регистрируем себя как текущий активный экземпляр
    {
        std::scoped_lock lock(s_mutex);
        s_instance = this;
        s_pendingCalls.clear();
    }

    // ── Загрузка библиотек ────────────────────────────────────────────────────
    if (!loadLibraries()) {
        // errorOccurred уже был emit-нут внутри loadLibraries()
        std::scoped_lock lock(s_mutex);
        s_instance = nullptr;
        return;
    }

    // ── Создание Plain-объекта ────────────────────────────────────────────────
    m_plain = m_fnPlainFactory();
    if (!m_plain) {
        emit errorOccurred(QStringLiteral("PlainRastrFactory returned nullptr"));
        std::scoped_lock lock(s_mutex);
        s_instance = nullptr;
        return;
    }

    // ── Загрузка данных ───────────────────────────────────────────────────────
    fs::path allPath = m_projectPath / "all";
    try {
        m_plain->Load(eLoadCode::RG_REPL, allPath.c_str(), "");
    } catch (const std::exception& ex) {
        emit errorOccurred(
            QStringLiteral("IPlainRastr::Load failed: %1").arg(ex.what()));
        std::scoped_lock lock(s_mutex);
        s_instance = nullptr;
        return;
    }
    emit statusMessage(
        QStringLiteral("Data loaded from: %1")
            .arg(QString::fromStdString(
                fs::weakly_canonical(allPath).string())));

    // ── Инициализация HTTP-сервера ────────────────────────────────────────────
    fs::path xmlPath = m_projectPath / "graph2libs.xml";
    m_fnInitPlainDLL(
        m_plain,
        xmlPath.c_str(),
        m_ip.toStdString().c_str(),
        m_port,
        &GraphicsWorker::staticAsyncCallback);

    // Сервер готов — уведомляем главный поток
    emit serverReady(serverUrl());
    emit statusMessage(
        QStringLiteral("Graphics server ready at %1").arg(serverUrl()));

    // ── Главный цикл диспетчеризации ──────────────────────────────────────────
    while (!m_stopRequested.loadRelaxed())
    {
        std::unique_lock lock(s_mutex);

        // Сначала обрабатываем всё накопленное
        while (!s_pendingCalls.empty())
        {
            auto it = s_pendingCalls.begin();
            int         iMSG   = it->first;
            std::string params = std::move(it->second);
            s_pendingCalls.erase(it);

            lock.unlock();               // отпускаем мьютекс на время обработки
            dispatchMessage(iMSG, params);
            lock.lock();                 // берём обратно перед следующей итерацией
        }

        if (m_stopRequested.loadRelaxed()) break;

        // Засыпаем до следующего callback или requestStop()
        s_cv.wait(lock);
    }

    emit statusMessage(QStringLiteral("GraphicsWorker stopped"));

    {
        std::scoped_lock lock(s_mutex);
        s_instance = nullptr;
    }
}
