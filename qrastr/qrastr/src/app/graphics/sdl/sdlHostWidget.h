#pragma once
#include <QWidget>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "elGraphService.h"
#include "test/SelfDrawingChild.h"

///@class Qt-виджет, встраивающий SDL-окно и SDL-рендер.
class SDLHostWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SDLHostWidget(QWidget* parent = nullptr);
    ~SDLHostWidget() override;

    /**
     * @brief Создать SDL-окно и рендер, инициализировать ElGraphService.
     * Должен вызываться ПОСЛЕ того, как виджет встроен в иерархию окон
     * (т.е. после addDockWidgetTab), иначе winId() ещё не назначен.
     */
    bool SDLInit();

    void onClose();

    /// Доступ к ElGraphService для GraphSDLManager (нужен для InitControl/CloseControl).
    ElGraphService&       elGraph()       { return m_elGraph; }
    const ElGraphService& elGraph() const { return m_elGraph; }

protected:
    void resizeEvent(QResizeEvent* event) override;

    // SDL рисует напрямую в нативное окно — Qt не должен перерисовывать поверх.
    QPaintEngine* paintEngine() const override { return nullptr; }

private slots:
    void performRendering();

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    QTimer*       m_timer    = nullptr;

    ElGraphService m_elGraph;
    // ТЕСТ: временное нативное окно вместо m_elGraph.
    // Рисует прямоугольник, меняющий цвет каждые 500 мс (GDI/Xlib).
    SelfDrawingChild m_testChild;
};