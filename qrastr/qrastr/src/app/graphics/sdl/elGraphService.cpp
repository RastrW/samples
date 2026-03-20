#include "elGraphService.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>

#if defined(Q_OS_WIN)
using InitPlainDLL_t = IPlainElGraph* (__cdecl*)();
#else
///@todo что делать на linux?
#endif

ElGraphService::ElGraphService(){
    const QString libDir = QCoreApplication::applicationDirPath() + "/";
    //Qt сам добавит .dll / .so / .dylib
    m_lib.setFileName(libDir + "ElGraphCtrl");
}

ElGraphService::~ElGraphService(){
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
    auto* initFn = reinterpret_cast<InitPlainDLL_t>(m_lib.resolve("InitPlainDLL"));
    if (!initFn) {
        spdlog::error("ElGraphService: символ InitPlainDLL не найден");
        m_lib.unload();
        return false;
    }
    // ── 3. Получаем указатель на интерфейс ───────────────────────────────────
    m_grc = initFn();
    if (!m_grc) {
        spdlog::error("ElGraphService: InitPlainDLL вернул nullptr");
        m_lib.unload();
        return false;
    }

    // ── 4. Встраиваем в нативное окно ───────────────────────────────────────
    // Сообщаем ElGraph его родительский HWND.
    // Реальный дочерний HWND будет создан позже — после InitControl.
    auto* res = m_grc->CreateChildWindow(parentHwnd);
    if (res) {
        if (res->Code() != IPlainElGraphRetCode::Ok)
            spdlog::error("ElGraphService: CreateChildWindow — {}",
                          res->What() ? res->What() : "?");
        res->Destroy();
    }

    spdlog::info("ElGraphService: ElGraphCtrl успешно инициализирован");
    return true;
}

void ElGraphService::fitToParent(void* parentHwnd) const
{
#if defined(Q_OS_WIN)
    HWND child;

    if (!m_grc){
        child = nullptr;
    }
    auto* res = m_grc->GetNativeHandle();
    if (!res) {
        child = nullptr;
    }
    HWND hwnd = reinterpret_cast<HWND>(static_cast<long long>(*res));
    res->Destroy();{
        child = hwnd;
    }

    HWND parent = reinterpret_cast<HWND>(parentHwnd);
    if (!child || !parent) return;

    RECT rc{};
    ::GetClientRect(parent, &rc);
    ::MoveWindow(child, 0, 0, rc.right, rc.bottom, TRUE);
    ::ShowWindow(child, SW_SHOW);
#endif
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
