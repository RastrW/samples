#pragma once
#include <QObject>
#include "GraphControlService.h"

namespace ads { class CDockManager; class CDockWidget; }

/**
 * @class Управляет всем жизненным циклом SDL-графики.
 *
 * Ответственности:
 *  - SDL_Init / SDL_Quit (один раз, счётчик открытых окон)
 *  - Загрузка GraphControlClient (живёт = менеджер)
 *  - Создание/удаление SDLChild-вкладок через CDockManager
*/
class GraphSDLManager : public QObject
{
    Q_OBJECT
public:
    explicit GraphSDLManager(ads::CDockManager* dockManager,
                             QWidget*           parentWidget,
                             IPlainRastr*       rastr,
                             QObject*           parent = nullptr);
    ~GraphSDLManager();

    /**
     * @brief Открыть новое SDL-окно графики во вкладке CDockManager.
     * SDL_Init вызывается автоматически при открытии первого окна.
     */
    void openWindow();
    /// Количество сейчас открытых SDL-окон.
    int openWindowCount() const { return m_windowCount; }
signals:
    /// успешное создания каждого нового окна.
    void windowOpened();
    /// закрытие каждого окна.
    void windowClosed();
    /// закрыто последнее окно (SDL остановлен).
    void allWindowsClosed();
private slots:
    /// Слот на сигнал CDockWidget::closed для каждого окна.
    void slot_dockClosed();
private:
    // ── Зависимости ─────────────────────────────────────────────────────────
    ads::CDockManager* m_dockManager  = nullptr;
    QWidget*           m_parentWidget = nullptr;

    // ── Состояние ───────────────────────────────────────────────────────────
    int  m_windowCount = 0;   ///< Счётчик открытых SDL-окон
    bool m_sdlInited   = false;

    // ── GraphControlClient — живёт пока живёт GraphSDLManager ───────────────
    std::unique_ptr<GraphControlService> m_gcc;

    /// Инициализировать SDL подсистему (вызывается при первом openWindow).
    bool ensureSDLInited();
    /// Остановить SDL (вызывается когда счётчик падает до 0).
    void shutdownSDLIfIdle();
};
