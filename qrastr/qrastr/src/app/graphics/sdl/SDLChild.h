#pragma once
#include <QWidget>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "elGraphService.h"

#if defined(Q_OS_WIN)
// Только объявления типов, без включения windows.h в заголовок
using HWINEVENTHOOK = struct HWINEVENTHOOK__*;
#endif

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

    /// Создать SDL-окно, рендер и дочернее окно ElGraph.
    /// Вызывать ПОСЛЕ того, как виджет показан (show + processEvents),
    /// иначе winId() ещё не назначен нативному окну.
    bool SDLInit();

    /// Разместить дочернее окно ElGraph по размеру виджета.
    /// Вызывать ПОСЛЕ initControl — именно тогда ElGraph создаёт HWND.
    void attachElGraph();

    void onClose();

    /// Доступ к ElGraphService для GraphSDLManager (нужен для InitControl/CloseControl).
    ElGraphService&       elGraph()       { return m_elGraph; }
    const ElGraphService& elGraph() const { return m_elGraph; }

protected:
    void resizeEvent(QResizeEvent* event) override;

    // SDL рисует напрямую в нативное окно — Qt не должен перерисовывать поверх.
    QPaintEngine* paintEngine() const override { return nullptr; }

private slots:
    void onRender();

private:
    void* nativeSDLHandle() const;

    SDL_Window*    m_window   = nullptr;
    SDL_Renderer*  m_renderer = nullptr;
    QTimer*        m_timer    = nullptr;
    ElGraphService m_elGraph;
};
