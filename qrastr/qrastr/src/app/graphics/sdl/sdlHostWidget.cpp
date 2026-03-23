#include "sdlHostWidget.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>
#include <spdlog/spdlog.h>

SDLHostWidget::SDLHostWidget(QWidget* parent)
    : QWidget(parent)
{
    // WA_NativeWindow: Qt создаёт настоящий HWND/XID для этого виджета,
    //                  без этого winId() может вернуть HWND родителя.
    setAttribute(Qt::WA_NativeWindow);

    // WA_PaintOnScreen: отключаем Qt-рисование поверх SDL-поверхности.
    setAttribute(Qt::WA_PaintOnScreen);

    setMinimumSize(100, 100);

    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);   // снижает дрожание частоты кадров
    connect(m_timer, &QTimer::timeout, this, &SDLHostWidget::performRendering);
}

SDLHostWidget::~SDLHostWidget()
{
    // ТЕСТ: уничтожаем тестовое дочернее окно до того, как SDL-окно исчезнет
    m_testChild.destroy();

    // Сначала убиваем ElGraph (дочернее окно)
    m_elGraph.shutdown();

    // Потом рендер и SDL-окно (родительское)
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
}

bool SDLHostWidget::SDLInit()
{
    // ── 1. Создаём SDL-окно, встроенное в нативный виджет ────────────────────
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty (props, SDL_PROP_WINDOW_CREATE_TITLE_STRING,       "Графика");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,  true);
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,       width());
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,      height());

    // SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN:
    // Говорит SDL не создавать собственный графический контекст заново,
    // а использовать уже существующий. Без этого флага SDL может
    // «пересоздать» контекст и затереть содержимое дочерних HWND.
    SDL_SetBooleanProperty(props,
                           SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN,
                           true);

#if defined(Q_OS_WIN)
    SDL_SetPointerProperty(props,
                           SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER,
                           reinterpret_cast<void*>(winId()));
#elif defined(Q_OS_LINUX)
    SDL_SetNumberProperty(props,
                          SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER,
                          static_cast<Sint64>(winId()));
#endif

    m_window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!m_window) {
        spdlog::error("SDLHostWidget::SDLInit: SDL_CreateWindowWithProperties — {}",
                      SDL_GetError());
        return false;
    }

// ── 2. Получаем HWND самого SDL-окна — он станет родителем для ElGraph ───
#if defined(Q_OS_WIN)
    void* sdlHwnd = SDL_GetPointerProperty(
        SDL_GetWindowProperties(m_window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        nullptr);
    spdlog::info("SDLHostWidget::SDLInit: SDL HWND    = 0x{:016X}",
                 reinterpret_cast<uintptr_t>(sdlHwnd));
    spdlog::info("SDLHostWidget::SDLInit: Qt  winId() = 0x{:016X}",
                 static_cast<quintptr>(winId()));
// Два значения могут совпасть — это нормально: SDL при встраивании в
// существующий HWND не создаёт новый, а оборачивает переданный winId().
// В таком случае иерархия фактически плоская и оба подхода эквивалентны.
#endif
    // ── 2. ТЕСТ: создаём SelfDrawingChild внутри SDL-окна ───────────────────
    // parentHandle — нативный HWND (Win) / XID (Linux) Qt-виджета.
    // SelfDrawingChild создаётся дочерним к этому же HWND, что и SDL-окно,
    // поэтому WS_CLIPCHILDREN выше не даёт SDL его затирать.
    // Дочернее окно позиционируется по центру виджета (четверть размера).
    // УДАЛИТЬ этот блок когда ElGraphService заработает.
    bool testWindow = true;
    if (testWindow) {
        if (!m_testChild.create(sdlHwnd,
                            width()  / 4, height() / 4,
                            width()  / 2, height() / 2)){
            spdlog::warn("SDLHostWidget::SDLInit: SelfDrawingChild::create завершился с ошибкой");
        }
    }

    // ── 3. Инициализируем ElGraphService ─────────────────────────────────────
    // ТЕСТ: пока m_testChild активен, ElGraphService не инициализируем,
    //       чтобы два дочерних окна не конфликтовали за одно пространство.
    //       Когда тест пройдёт — убрать условие и оставить только m_elGraph.init().
    if (!testWindow) {
        if (!m_elGraph.init(sdlHwnd)) {
            spdlog::warn("SDLHostWidget::SDLInit: ElGraphCtrl недоступен");
        }
    }

    // ── 4. Создаём рендер ────────────────────────────────────────────────────
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        spdlog::error("SDLHostWidget::SDLInit: SDL_CreateRenderer — {}", SDL_GetError());
        return false;
    }

    spdlog::info("SDLHostWidget: renderer = {}", SDL_GetRendererName(m_renderer));
    m_timer->start(1000 / 60);
    spdlog::info("SDLHostWidget::SDLInit: OK, окно {}x{}", width(), height());
    return true;
}

void SDLHostWidget::onClose()
{
    m_timer->stop();

    // ТЕСТ: останавливаем тестовое окно
    m_testChild.destroy();

    m_elGraph.shutdown();
}

/*
void SDLHostWidget::performRendering()
{
    SDL_PumpEvents();
    if (!m_renderer) return;

    SDL_SetRenderDrawColor(m_renderer, 200, 50, 50, SDL_ALPHA_OPAQUE); // красный
    SDL_RenderClear(m_renderer);

    // Тест: рисуем белый прямоугольник через SDL
    SDL_SetRenderDrawColor(m_renderer, 222, 21, 23, SDL_ALPHA_OPAQUE);
    SDL_FRect rect = {50.f, 50.f, 200.f, 100.f};
    SDL_RenderFillRect(m_renderer, &rect);

    SDL_RenderPresent(m_renderer);
}
//*/

///*
void SDLHostWidget::performRendering(){

    SDL_PumpEvents();

    if (!m_renderer) return;
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer);

    SDL_RenderPresent(m_renderer);
}
//*/

void SDLHostWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_window) {
        SDL_SetWindowSize(m_window, event->size().width(), event->size().height());
    }

    // ТЕСТ: при ресайзе держим SelfDrawingChild по центру виджета
    m_testChild.resize(event->size().width()  / 4, event->size().height() / 4,
                   event->size().width()  / 2, event->size().height() / 2);
}