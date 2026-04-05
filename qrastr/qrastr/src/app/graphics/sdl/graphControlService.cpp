#include "graphControlService.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <QFile>
#include <QApplication>
#include "astra/IPlainRastr.h"
#include "IPlainElGraph.h"

GraphControlService::GraphControlService(IPlainRastr* rastr):
    m_rastr(rastr){
    const QString libDir = QCoreApplication::applicationDirPath() + "/";
    m_lib.setFileName(libDir + "GraphClient");
}

GraphControlService::~GraphControlService() {
    m_initPlainDll(nullptr, "");

    m_loaded       = false;

    spdlog::info("before unload");
    ///@todo если не включить выгрузку библиотеки, то процесс завершения навсегда зависнет.
    /// если включить выгрузку, то аварийно завершится.
    /// Скорее всего связано с тем, что не были завершена внутренние потоки в dll
    /// Примечание: зависание происходит только тогда
    unload();

    spdlog::info("after unload");

    spdlog::info("GraphControlService has been deleted");
}

bool GraphControlService::load()
{
    if (m_loaded) return true;

    if (!m_lib.load()) {
        spdlog::error("GraphControlService: не удалось загрузить GraphClient — {}",
                      m_lib.errorString().toStdString());
        return false;
    }

    m_initControl  = reinterpret_cast<GCC_InitControl_t> (m_lib.resolve("InitControl"));
    m_closeControl = reinterpret_cast<GCC_CloseControl_t>(m_lib.resolve("CloseControl"));
    m_initPlainDll = reinterpret_cast<GCC_InitPlainDLL_t> (m_lib.resolve("InitPlainDLL"));

    if (!m_initControl || !m_closeControl || !m_initPlainDll) {
        spdlog::error("GraphControlService: не найдены экспорты InitControl/CloseControl");
        m_lib.unload();
        return false;
    }

    const QString graphLibsPath =
        QCoreApplication::applicationDirPath() + "/../Data/graphics/graph2libs.xml";
    if (!QFile::exists(graphLibsPath)) {
        spdlog::warn("GraphWorker: graph2libs.xml not found");
        return false;
    }

    m_initPlainDll(m_rastr, graphLibsPath.toStdString().c_str());
    m_loaded = true;
    spdlog::info("GraphControlService: GraphControlClient загружен");
    return true;
}

void GraphControlService::unload()
{

    m_initControl  = nullptr;
    m_closeControl = nullptr;
    m_initPlainDll = nullptr;

    if (m_lib.isLoaded()) {
        if (m_lib.unload()){
            spdlog::info("GraphControlService: GraphControlClient выгружен");
        }else{
            spdlog::error("GraphControlService: Возникла проблема с выгрузкой библиотеки GraphControlClient");
        }
    }
    m_loaded = false;
}

void GraphControlService::initControl(IPlainElGraph* graph)
{
    if (!graph) {
        spdlog::warn("GraphControlService::initControl: pcontrol == nullptr, пропускаем");
        return;
    }
    if (!m_initControl) {
        spdlog::warn("GraphControlService::initControl: GCC не загружен");
        return;
    }

    const QString graphLibsPath =
        QCoreApplication::applicationDirPath() + "/../Data/graphics/graph2items.xml";
    if (!QFile::exists(graphLibsPath)) {
        spdlog::warn("GraphWorker: graph2items.xml not found");
        return;
    }

    auto* dpcResult = graph->GetDrawShapesCollection();
    if (dpcResult) {
        auto* dpc = dpcResult->operator->(); // теперь это IPlainElGraphDrawShapesCollection*
        if (dpc) {
            auto* res = dpc->LoadShapes(graphLibsPath.toStdString());
            spdlog::warn("graph2items.xml is loaded");
            if (res) res->Destroy();   // освобождаем результат LoadShapes
        }
        dpcResult->Destroy(); // освобождаем сам result object
    }

    //m_initControl(graph, m_rastr);
    m_initControl(graph, nullptr);
}

void GraphControlService::closeControl(IPlainElGraph* graph)
{
    if (!graph) return;
    if (!m_closeControl) {
        spdlog::warn("GraphControlService::closeControl: GCC не загружен");
        return;
    }
    m_closeControl(graph);
}