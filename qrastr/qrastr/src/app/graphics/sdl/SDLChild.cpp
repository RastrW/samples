#include "SDLChild.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>
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
    connect(m_timer, &QTimer::timeout, this, &SDLChild::Render);
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
    // ── 1. Создаём SDL-окно, встроенное в нативный виджет ────────────────────
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty (props, SDL_PROP_WINDOW_CREATE_TITLE_STRING,       "Графика");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,  true);
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,       width());
    SDL_SetNumberProperty (props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,      height());

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
        spdlog::error("SDLChild::SDLInit: SDL_CreateWindowWithProperties — {}",
                      SDL_GetError());
        return false;
    }

    // ── 2. Инициализируем ElGraphService ─────────────────────────────────────
    //Создаём нативный HWND, чтобы встроить в него SDL
    void* nativeHandle = getSDLNativeHandle();

    if (!m_elGraph.init(nativeHandle)) {
        spdlog::warn("SDLChild::SDLInit: ElGraphCtrl недоступен");
    }

    // ── 3. Создаём рендер ────────────────────────────────────────────────────
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        spdlog::error("SDLChild::SDLInit: SDL_CreateRenderer — {}", SDL_GetError());
        return false;
    }

    m_timer->start(1000 / 60);   // ~60 fps
    spdlog::info("SDLChild::SDLInit: OK, окно {}x{}", width(), height());
    return true;
}

void SDLChild::OnClose()
{
    m_timer->stop();
    m_elGraph.shutdown();
}

void SDLChild::Render()
{
    SDL_PumpEvents();

    if (!m_renderer) return;
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer);
    //Если нужно рисовать что-то своё средствами
    //SDL вокруг или поверх ElGraph — это место для таких вызовов.

    SDL_RenderPresent(m_renderer);
}

void SDLChild::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Синхронизируем размер SDL-окна с Qt-виджетом
    if (m_window) {
        SDL_SetWindowSize(m_window, event->size().width(), event->size().height());
    }
    ///@todo возожно, ElGraph должен быть уведомлен о смене размера
}

void* SDLChild::getSDLNativeHandle() const
{
#if defined(Q_OS_WIN)
    return SDL_GetPointerProperty(
        SDL_GetWindowProperties(m_window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        nullptr);
#elif defined(Q_OS_LINUX)
    // Возвращаем XID как указатель; ElGraphService::init должен привести обратно.
    auto xid = static_cast<Sint64>(
        SDL_GetNumberProperty(SDL_GetWindowProperties(m_window),
                              SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
    return reinterpret_cast<void*>(static_cast<uintptr_t>(xid));
#else
    return nullptr;
#endif
}