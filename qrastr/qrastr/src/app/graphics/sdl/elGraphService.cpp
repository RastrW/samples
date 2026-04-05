#include "elGraphService.h"
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include "IPlainElGraph.h"

#if defined(Q_OS_WIN)
using InitPlainDLL_t = IPlainElGraph* (__cdecl*)();
#else
using InitPlainDLL_t = IPlainElGraph* (*)();
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
    // Логируем HWND в hex для сравнения с тем, что передаёт SelfDrawingChild.
    spdlog::info("ElGraphService::init: parentHwnd = 0x{:016X}",
                 reinterpret_cast<uintptr_t>(parentHwnd));
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

void ElGraphService::shutdown()
{
    if (m_grc) {
        //m_grc->Destroy();
        m_grc = nullptr;
    }
    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}

void ElGraphService::destroyWindow(){
    //m_grc->DestroyWindow();
}
