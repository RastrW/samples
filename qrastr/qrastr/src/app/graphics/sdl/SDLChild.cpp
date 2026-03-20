#include "SDLChild.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QResizeEvent>
#include <spdlog/spdlog.h>

#if defined(Q_OS_WIN)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <QTimer>
#include <QResizeEvent>
#include <QApplication>
#include <spdlog/spdlog.h>

SDLChild::SDLChild(QWidget* parent)
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
    connect(m_timer, &QTimer::timeout, this, &SDLChild::onRender);
}

SDLChild::~SDLChild(){
    // 1. Сначала убиваем ElGraph (дочернее окно)
    m_elGraph.shutdown();

    // 2. Потом рендер и SDL-окно (родительское)
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window   = nullptr; }
}

bool SDLChild::SDLInit()
{
    // 1. SDL-окно встроенное в нативный Qt-виджет
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty (props, SDL_PROP_WINDOW_CREATE_TITLE_STRING,      "Графика");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,      width());
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,     height());
#if defined(Q_OS_WIN)
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER,
                           reinterpret_cast<void*>(winId()));
#elif defined(Q_OS_LINUX)
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER,
                          static_cast<Sint64>(winId()));
#endif
    m_window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!m_window) {
        spdlog::error("SDLChild: SDL_CreateWindowWithProperties — {}", SDL_GetError());
        return false;
    }

#if defined(Q_OS_WIN)
    // WS_CLIPCHILDREN — не рисовать SDL поверх дочернего окна ElGraph
    HWND hwnd = reinterpret_cast<HWND>(winId());
    LONG_PTR style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
    ::SetWindowLongPtr(hwnd, GWL_STYLE, style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    spdlog::info("SDLChild: style after  = {:x}", (uintptr_t)style);
#endif

    // 2. ElGraph — CreateChildWindow даёт ему родительский HWND.
    //    Реальный HWND ElGraph создаст позже, после InitControl.
    if (!m_elGraph.init(nativeSDLHandle()))
        spdlog::warn("SDLChild: ElGraphCtrl недоступен");

    // 3. Рендер
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        spdlog::error("SDLChild: SDL_CreateRenderer — {}", SDL_GetError());
        return false;
    }
    spdlog::info("SDLChild: renderer = {}", SDL_GetRendererName(m_renderer));
    m_timer->start(1000 / 60);// ~60 fps
    spdlog::info("SDLChild: инициализирован {}x{}", width(), height());
    return true;
}

void SDLChild::attachElGraph()
{
    // Вызывать после initControl — к этому моменту ElGraph уже создал свой HWND.
    m_elGraph.fitToParent(nativeSDLHandle());
}

void SDLChild::onClose()
{
    m_timer->stop();
    m_elGraph.shutdown();
}

void SDLChild::onRender()
{
    SDL_PumpEvents();
    if (!m_renderer) return;

    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer);
    // Место для SDL-примитивов вокруг ElGraph, если нужно
    SDL_RenderPresent(m_renderer);
}

/*
void SDLChild::Render()
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
*/

void SDLChild::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_window)
        SDL_SetWindowSize(m_window, event->size().width(), event->size().height());
    m_elGraph.fitToParent(nativeSDLHandle()); // безопасно до init — проверяет сам
}

void* SDLChild::nativeSDLHandle() const
{
#if defined(Q_OS_WIN)
    return SDL_GetPointerProperty(SDL_GetWindowProperties(m_window),
                                  SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(Q_OS_LINUX)
    auto xid = static_cast<Sint64>(
        SDL_GetNumberProperty(SDL_GetWindowProperties(m_window),
                              SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
    return reinterpret_cast<void*>(static_cast<uintptr_t>(xid));
#else
    return nullptr;
#endif
}
