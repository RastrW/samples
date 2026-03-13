#include "SDLChild.h"
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>

SDLChild::SDLChild(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow);   // нужен нативный HWND/XID
    setAttribute(Qt::WA_PaintOnScreen);  // отключаем Qt-рисование поверх
    setMinimumSize(100, 100);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SDLChild::Render);
}

SDLChild::~SDLChild() {
    m_timer->stop();
    if (m_texture)  SDL_DestroyTexture(m_texture);
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}

SDL_AppResult SDL_Fail(){
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}
SDL_AppResult SDL_OK(){
    return SDL_APP_SUCCESS;
}

SDL_AppResult SDLChild::SDLInit() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        qWarning() << "SDL_Init:" << SDL_GetError();
        return SDL_Fail();
    }

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

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        qWarning() << "SDL_CreateRenderer:" << SDL_GetError();
        return SDL_Fail();
    }

    // Загрузка SVG через временный файл
    QFile f(":/images/cx195.svg");
    QString tmp = QDir::tempPath() + "/cx195_copy.svg";
    QFile::remove(tmp);
    f.copy(tmp);

    SDL_Surface* surf = IMG_Load(tmp.toStdString().c_str());
    if (!surf) {
        qWarning() << "IMG_Load:" << SDL_GetError();
        return SDL_Fail();
    }
    m_texture = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_DestroySurface(surf);
    if (!m_texture) return SDL_Fail();

    m_timer->start(1000 / 60);

    return SDL_OK();
}

void SDLChild::Render() {
    if (!m_renderer || !m_texture) return;

    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(m_renderer);

    // NULL, NULL → растягивать текстуру на весь viewport (масштабирование!)
    SDL_RenderTexture(m_renderer, m_texture, nullptr, nullptr);

    SDL_RenderPresent(m_renderer);
}

void SDLChild::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // Синхронизируем размер SDL-окна с Qt-виджетом
    if (m_window) {
        SDL_SetWindowSize(m_window, event->size().width(), event->size().height());
    }
}

void SDLChild::OnClose()
{
    m_timer->stop();
}
