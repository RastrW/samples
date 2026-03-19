#include "elGraphService.h"
#include <spdlog/spdlog.h>

#if defined(Q_OS_WIN)
using InitPlainDLL_t = IPlainElGraph* (__cdecl*)();
#else
///@todo что делать на linux?
#endif

ElGraphService::~ElGraphService()
{
    shutdown();
}

bool ElGraphService::init(void* parentHwnd)
{
    if (m_grc) {
        spdlog::warn("ElGraphService::init: уже инициализирован, повторный вызов проигнорирован");
        return true;
    }

    // ── 1. Загружаем DLL/SO ──────────────────────────────────────────────────
    if (!m_lib.load()) {
        spdlog::error("ElGraphService: не удалось загрузить ElGraphCtrl — {}",
                      m_lib.errorString().toStdString());
        return false;
    }

    // ── 2. Резолвим точку входа ──────────────────────────────────────────────
    auto f = reinterpret_cast<InitPlainDLL_t>(m_lib.resolve("InitPlainDLL"));
    if (!f) {
        spdlog::error("ElGraphService: символ InitPlainDLL не найден в ElGraphCtrl");
        m_lib.unload();
        return false;
    }

    // ── 3. Получаем указатель на интерфейс ───────────────────────────────────
    m_grc = f();
    if (!m_grc) {
        spdlog::error("ElGraphService: InitPlainDLL вернул nullptr");
        m_lib.unload();
        return false;
    }

    // ── 4. Встраиваем в нативное окно ───────────────────────────────────────
    // CreateChildWindow возвращает IPlainElGraphResult*
    IPlainElGraphResult* res = m_grc->CreateChildWindow(parentHwnd);
    if (res) {
        if (res->Code() != IPlainElGraphRetCode::Ok) {
            spdlog::error("ElGraphService: CreateChildWindow вернул ошибку — {}",
                          res->What() ? res->What() : "(нет сообщения)");
        }
        res->Destroy();
    }

    spdlog::info("ElGraphService: ElGraphCtrl успешно инициализирован");
    return true;
}

void ElGraphService::shutdown()
{
    if (m_grc) {
        m_grc->Destroy();
        m_grc = nullptr;
    }
    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}
