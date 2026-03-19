#include "graphControlService.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include "astra/IPlainRastr.h"

GraphControlService::GraphControlService(IPlainRastr* rastr):
    m_rastr(rastr){
    const QString libDir = QCoreApplication::applicationDirPath() + "/";
    m_lib.setFileName(libDir + "GraphClient");
}

GraphControlService::~GraphControlService() {
    unload();
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

    m_initPlainDll(m_rastr,
                   "",
                   "", 0,
                   nullptr);
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
        m_lib.unload();
        spdlog::info("GraphControlService: GraphControlClient выгружен");
    }
    m_loaded = false;
}

void GraphControlService::initControl(IPlainElGraph* pcontrol)
{
    if (!pcontrol) {
        spdlog::warn("GraphControlService::initControl: pcontrol == nullptr, пропускаем");
        return;
    }
    if (!m_initControl) {
        spdlog::warn("GraphControlService::initControl: GCC не загружен");
        return;
    }
    m_initControl(pcontrol, nullptr);
    spdlog::debug("GraphControlService::initControl: подписка установлена для {:p}",
                  static_cast<void*>(pcontrol));
}

void GraphControlService::closeControl(IPlainElGraph* pcontrol)
{
    if (!pcontrol) return;
    if (!m_closeControl) {
        spdlog::warn("GraphControlService::closeControl: GCC не загружен");
        return;
    }
    m_closeControl(pcontrol);
    spdlog::debug("GraphControlService::closeControl: подписка снята для {:p}",
                  static_cast<void*>(pcontrol));
}