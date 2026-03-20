#pragma once
#include <QObject>
#include "GraphControlService.h"
#include "iGraphManager.h"

namespace ads { class CDockManager; class CDockWidget; }

/**
 * @class Управляет всем жизненным циклом SDL-графики.
 *
 * Ответственности:
 *  - SDL_Init / SDL_Quit (один раз, счётчик открытых окон)
 *  - Загрузка GraphControlClient (живёт = менеджер)
 *  - Создание/удаление SDLChild-вкладок через CDockManager
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
    IPlainRastr* m_rastr       = nullptr;
    int          m_windowCount = 0; ///< Счётчик открытых SDL-окон
    bool         m_sdlInited   = false;
    std::unique_ptr<GraphControlService> m_gcc;
    /// Инициализировать SDL подсистему (вызывается при первом openWindow).
    bool ensureSDLInited();
    /// Остановить SDL (вызывается когда счётчик падает до 0).
    void shutdownSDLIfIdle();
};
