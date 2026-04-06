#include "graphSDLManager.h"
#include "sdlHostWidget.h"
#include "hostWidget.h"
#include <DockManager.h>
#include <DockWidget.h>
#include <QApplication>
#include <spdlog/spdlog.h>
#include <QWidget>
#include "graphControlService.h"

GraphSDLManager::GraphSDLManager(ads::CDockManager* dockManager,
                                 QWidget*           parentWidget,
                                 IPlainRastr*       rastr,
                                 QObject*           parent)
    : IGraphManager(dockManager, parentWidget, parent)
    , m_rastr(rastr){
    m_gcc = std::make_unique<GraphControlService>(rastr);
    //if (!m_gcc->load())
    //   spdlog::warn("GraphSDLManager: GraphControlClient недоступен");
}

GraphSDLManager::~GraphSDLManager()
{
    // Если окна остались открытыми — останавливаем SDL.
    if (m_sdlInited && m_windowWithSDL) {
        SDL_Quit();
        m_sdlInited = false;
        spdlog::warn("GraphSDLManager::~GraphSDLManager: SDL_Quit в деструкторе, "
                     "остались незакрытые окна ({} шт.)", m_windowCount);
    }

    spdlog::info("GraphSDLManager has been deleted");
}

void GraphSDLManager::closeAll(){
    if (m_sdlInited && m_windowWithSDL) {
        SDL_Quit();
        m_sdlInited = false;
    }
}

void GraphSDLManager::openWindow()
{

    // ── 1. Создаём dock-виджет и SDLHostWidget ────────────────────────────────────
    auto* dw       = new ads::CDockWidget(tr("Графика SDL"), m_parentWidget);

    if (!m_windowWithSDL) {
        auto* hostWidget = new HostWidget(dw);

        dw->hide();
        dw->setWidget(hostWidget);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        // ── 3. Встраиваем в dock ─────────────────────────────────────────────────
        m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        dw->show();  // ← сначала показываем

        // ← SDLInit ПОСЛЕ show(), чтобы HWND был видимым когда ElGraph его получит
        QApplication::processEvents();  // дать Qt обработать show()
        // ── 4. SDL-окно инициализируется ПОСЛЕ встраивания виджета в иерархию,
        //       иначе winId() ещё не назначен нативному окну ─────────────────────
        if (!hostWidget->init()) {
            // Не возвращаемся — dock уже создан, пусть хотя бы закроется штатно.
            spdlog::error("GraphSDLManager: HostWidget::HostWidget завершился с ошибкой");
        } else {
            // ── 5. InitControl — после успешного CreateChildWindow ────────────────
            //       Устанавливает подписку GraphControlClient на хинты ElGraph.
            spdlog::info("GraphControlService initControl");
            m_gcc->initControl(hostWidget->elGraph().graph());
        }
        dw->show();
        // Сохраняем указатель на sdlHostWidget внутри dock-виджета через свойство,
        // чтобы в слоте onDockClosed можно было его получить без lambda-захвата.
        // Используем динамическое свойство Qt.
        dw->setProperty("HostWidget", QVariant::fromValue(static_cast<QObject*>(hostWidget)));
    }else {
        // ── SDL_Init — только при первом открытии окна ────────────────────────
        if (!ensureSDLInited()) return;
        auto* sdlHostWidget = new SDLHostWidget(dw);

        dw->hide();
        dw->setWidget(sdlHostWidget);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        // ── 3. Встраиваем в dock ─────────────────────────────────────────────────
        m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        dw->show();  // ← сначала показываем

        // ← SDLInit ПОСЛЕ show(), чтобы HWND был видимым когда ElGraph его получит
        QApplication::processEvents();  // дать Qt обработать show()
        // ── 4. SDL-окно инициализируется ПОСЛЕ встраивания виджета в иерархию,
        //       иначе winId() ещё не назначен нативному окну ─────────────────────
        if (!sdlHostWidget->SDLInit()) {
            // Не возвращаемся — dock уже создан, пусть хотя бы закроется штатно.
            spdlog::error("GraphSDLManager: SDLHostWidget::SDLHostWidget завершился с ошибкой");
        } else {
            // ── 5. InitControl — после успешного CreateChildWindow ────────────────
            //       Устанавливает подписку GraphControlClient на хинты ElGraph.
            spdlog::info("GraphControlService initControl");
            m_gcc->initControl(sdlHostWidget->elGraph().graph());
        }
        dw->show();
        // Сохраняем указатель на sdlHostWidget внутри dock-виджета через свойство,
        // чтобы в слоте onDockClosed можно было его получить без lambda-захвата.
        // Используем динамическое свойство Qt.
        dw->setProperty("SDLHostWidget", QVariant::fromValue(static_cast<QObject*>(sdlHostWidget)));
    }

    // ── 6. При закрытии dock-виджета: CloseControl → OnClose → счётчик ───────
    connect(dw, &ads::CDockWidget::closed,
            this, &GraphSDLManager::slot_dockClosed);

    m_windowCount++;
    spdlog::info("GraphSDLManager: открыто окно #{}", m_windowCount);

    emit windowOpened(dw);
}

void GraphSDLManager::slot_dockClosed()
{
    auto* dw = qobject_cast<ads::CDockWidget*>(sender());
    if (!dw) return;
    if (m_windowWithSDL){
        auto* sdlHostWidgetObj = dw->property("SDLHostWidget").value<QObject*>();
        auto* sdlHostWidget    = qobject_cast<SDLHostWidget*>(sdlHostWidgetObj);

        if (sdlHostWidget) {

            // 1. CloseControl — отписываем GraphControlClient от хинтов ElGraph
            m_gcc->closeControl(sdlHostWidget->elGraph().graph());

            // 2. Останавливаем рендер-таймер, даём SDLHostWidget корректно завершиться
            sdlHostWidget->onClose();
        }
    }else{
        auto* hostWidgetObj = dw->property("HostWidget").value<QObject*>();
        auto* hostWidget    = qobject_cast<HostWidget*>(hostWidgetObj);

        if (hostWidget) {
            hostWidget->elGraph().destroyWindow();
            // 1. CloseControl — отписываем GraphControlClient от хинтов ElGraph
            m_gcc->closeControl(hostWidget->elGraph().graph());

            // 2. Останавливаем рендер-таймер, даём HostWidget корректно завершиться
            hostWidget->onClose();
        }
    }

    m_windowCount--;
    spdlog::info("GraphSDLManager: закрыто окно, осталось {}", m_windowCount);
    emit windowClosed();

    // ── SDL_Quit только когда закрыто последнее окно ─────────────────────────
    if (m_windowWithSDL){
        shutdownSDLIfIdle();
    }
}

bool GraphSDLManager::ensureSDLInited()
{
    if (m_sdlInited && m_windowWithSDL) return true;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        spdlog::error("GraphSDLManager: SDL_Init(VIDEO) failed — {}", SDL_GetError());
        return false;
    }

    m_sdlInited = true;
    spdlog::info("GraphSDLManager: SDL инициализирован");
    return true;
}

void GraphSDLManager::shutdownSDLIfIdle()
{
    if (!m_sdlInited)      return;

    SDL_Quit();
    m_sdlInited = false;
    spdlog::info("GraphSDLManager: SDL остановлен (нет открытых окон)");
    emit allWindowsClosed();
}
