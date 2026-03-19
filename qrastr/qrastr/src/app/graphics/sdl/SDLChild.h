#pragma once
#include <QWidget>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "elGraphService.h"

class ElGraphService;

/**
 * @class Qt-виджет, встраивающий SDL-окно и SDL-рендер.
 *
 * НЕ вызывает SDL_Init / SDL_Quit — это ответственность GraphSDLManager.
 * Владеет одним ElGraphService (один экземпляр IPlainElGraph на окно).
 */
class SDLChild : public QWidget
{
    Q_OBJECT
public:
    explicit SDLChild(QWidget* parent = nullptr);
    ~SDLChild() override;

    /**
     * @brief Создать SDL-окно и рендер, инициализировать ElGraphService.
     * Должен вызываться ПОСЛЕ того, как виджет встроен в иерархию окон
     * (т.е. после addDockWidgetTab), иначе winId() ещё не назначен.
     */
    bool SDLInit();

    void OnClose();

    /// Доступ к ElGraphService для GraphSDLManager (нужен для InitControl/CloseControl).
    ElGraphService&       elGraph()       { return m_elGraph; }
    const ElGraphService& elGraph() const { return m_elGraph; }

protected:
    void resizeEvent(QResizeEvent* event) override;

    // SDL рисует напрямую в нативное окно — Qt не должен перерисовывать поверх.
    QPaintEngine* paintEngine() const override { return nullptr; }

private slots:
    void Render();

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    QTimer*       m_timer    = nullptr;

    ElGraphService m_elGraph;

    /// Получить нативный HWND/XID SDL-окна (для ElGraphService::init).
    void* getSDLNativeHandle() const;
};
