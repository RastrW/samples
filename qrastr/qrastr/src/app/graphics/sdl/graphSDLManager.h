#pragma once
#include <memory>
#include "iGraphManager.h"

namespace ads { class CDockManager; class CDockWidget; }
class IPlainRastr;
class GraphControlService;
/**
 * @class Управляет всем жизненным циклом SDL-графики.
 *
 * Ответственности:
 *  - SDL_Init / SDL_Quit (один раз, счётчик открытых окон)
 *  - Загрузка GraphControlClient (живёт = менеджер)
 *  - Создание/удаление SDLHostWidget-вкладок через CDockManager
    Объявление функций в библиотеках:
    GraphClient:
    extern "C" SBtype void InitPlainDLL(IPlainRastr* pPLain, const char* libpath)
    extern "C" SBtype void InitControl(long** pControl, IPlainRastr* pIsolatedPLain);
    extern "C" SBtype void CloseControl(long** pControl);

    ElGraph:
    extern "C" SBtype IPlainElGraph *InitPlainDLL()

    ,где SBtype:
    #ifdef _WIN32
        #define SBtype __declspec(dllexport)
    #else
        #define SBtype __attribute__((visibility("default")))
    #endif
*/
class GraphSDLManager : public IGraphManager
{
    Q_OBJECT
public:
    explicit GraphSDLManager(ads::CDockManager* dockManager,
                             QWidget*           parentWidget,
                             IPlainRastr*       rastr,
                             QObject*           parent = nullptr);
    ~GraphSDLManager() override;

    void openWindow() override;
    void closeAll()   override;
    int  openWindowCount() const override { return m_windowCount; }
private slots:
    void slot_dockClosed();

private:
    int          m_windowCount = 0; ///< Счётчик открытых SDL-окон
    bool         m_sdlInited   = false;
    std::unique_ptr<GraphControlService> m_gcc;
    /// Инициализировать SDL подсистему (вызывается при первом openWindow).
    bool ensureSDLInited();
    /// Остановить SDL (вызывается когда счётчик падает до 0).
    void shutdownSDLIfIdle();
    bool m_windowWithSDL {false};
};
