#include "SDLChild.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>
#include <spdlog/spdlog.h>

SDLChild::SDLChild(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);   // нужен нативный HWND/XID
    setAttribute(Qt::WA_PaintOnScreen);  // отключаем Qt-рисование поверх
    setMinimumSize(100, 100);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SDLChild::Render);
}

SDLChild::~SDLChild() {
    m_timer->stop();

    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}

SDL_AppResult SDLChild::SDLInit() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        qWarning() << "SDL_Init:" << SDL_GetError();
        return SDL_Fail();
    }
    // ── 1. Создаём SDL-окно, прикреплённое к нативному виджету ───────────────
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props,  SDL_PROP_WINDOW_CREATE_TITLE_STRING,    "Графика");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetNumberProperty(props,  SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,    width());
    SDL_SetNumberProperty(props,  SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,   height());

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
        qWarning() << "SDL_CreateWindow:" << SDL_GetError();
        return SDL_Fail();
    }

    // ── 2. Инициализируем ElGraphCtrl ────────────────────────────────────────
    // HWND должен быть получен из уже созданного SDL-окна, а не из winId():
    // SDL могла создать собственное нативное окно поверх виджета.
    void* nativeHwnd = nullptr;
#if defined(Q_OS_WIN)
    nativeHwnd = SDL_GetPointerProperty(
        SDL_GetWindowProperties(m_window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        nullptr);
#endif

    // Неудача не прерывает запуск: SDL-окно и рендер работают независимо
    if (!m_elGraph.init(nativeHwnd)) {
        spdlog::warn("SDLChild: ElGraphCtrl недоступен, продолжаем без него");
    }
    // ── 3. Создаём рендер ────────────────────────────────────────────────────
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        qWarning() << "SDL_CreateRenderer:" << SDL_GetError();
        return SDL_Fail();
    }

    m_timer->start(1000 / 60);

    return SDL_OK();
}

void SDLChild::Render() {
    if (!m_renderer) return;

    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer);

    SDL_RenderPresent(m_renderer);
}

void SDLChild::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // Синхронизируем размер SDL-окна с Qt-виджетом
    if (m_window) {
        SDL_SetWindowSize(m_window, event->size().width(), event->size().height());
    }
}

void SDLChild::OnClose(){

    m_timer->stop();
}

SDL_AppResult SDLChild::SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDLChild::SDL_OK(){
    return SDL_APP_SUCCESS;
}
