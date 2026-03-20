#include "GraphSDLManager.h"
#include "SDLChild.h"

#include <DockManager.h>
#include <DockWidget.h>

#include <spdlog/spdlog.h>
#include <QWidget>

GraphSDLManager::GraphSDLManager(ads::CDockManager* dockManager,
                                 QWidget*           parentWidget,
                                 IPlainRastr*       rastr,
                                 QObject*           parent)
    : IGraphManager(dockManager, parentWidget, parent)
    , m_rastr(rastr){
    m_gcc = std::make_unique<GraphControlService>(rastr);
    if (!m_gcc->load())
        spdlog::warn("GraphSDLManager: GraphControlClient недоступен");
}

GraphSDLManager::~GraphSDLManager()
{
    // Если окна остались открытыми — останавливаем SDL.
    if (m_sdlInited) {
        SDL_Quit();
        m_sdlInited = false;
        spdlog::warn("GraphSDLManager::~GraphSDLManager: SDL_Quit в деструкторе, "
                     "остались незакрытые окна ({} шт.)", m_windowCount);
    }
}

void GraphSDLManager::closeAll(){
    if (m_sdlInited) {
        SDL_Quit();
        m_sdlInited = false;
    }
}

void GraphSDLManager::openWindow()
{
    // ── 1. SDL_Init — только при первом открытии окна ────────────────────────
    if (!ensureSDLInited()) return;
    // ── 2. Создаём dock-виджет и SDLChild ────────────────────────────────────
    auto* dw       = new ads::CDockWidget(tr("Графика SDL"), m_parentWidget);
    auto* sdlChild = new SDLChild(dw);
    dw->hide();
    dw->setWidget(sdlChild);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    // ── 3. Встраиваем в dock ─────────────────────────────────────────────────
    m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
    // ── 4. SDL-окно инициализируется ПОСЛЕ встраивания виджета в иерархию,
    //       иначе winId() ещё не назначен нативному окну ─────────────────────
    if (!sdlChild->SDLInit()) {
        // Не возвращаемся — dock уже создан, пусть хотя бы закроется штатно.
        spdlog::error("GraphSDLManager: SDLChild::SDLInit завершился с ошибкой");
    } else {
        // ── 5. InitControl — после успешного CreateChildWindow ────────────────
        //       Устанавливает подписку GraphControlClient на хинты ElGraph.
        m_gcc->initControl(sdlChild->elGraph().graph());
    }

    dw->show();
    // Сохраняем указатель на sdlChild внутри dock-виджета через свойство,
    // чтобы в слоте onDockClosed можно было его получить без lambda-захвата.
    // Используем динамическое свойство Qt.
    dw->setProperty("sdlChild", QVariant::fromValue(static_cast<QObject*>(sdlChild)));
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

    auto* sdlChildObj = dw->property("sdlChild").value<QObject*>();
    auto* sdlChild    = qobject_cast<SDLChild*>(sdlChildObj);

    if (sdlChild) {
        // 1. CloseControl — отписываем GraphControlClient от хинтов ElGraph
        m_gcc->closeControl(sdlChild->elGraph().graph());

        // 2. Останавливаем рендер-таймер, даём SDLChild корректно завершиться
        sdlChild->OnClose();
    }

    m_windowCount--;
    spdlog::info("GraphSDLManager: закрыто окно, осталось {}", m_windowCount);
    emit windowClosed();

    // ── SDL_Quit только когда закрыто последнее окно ─────────────────────────
    shutdownSDLIfIdle();
}

bool GraphSDLManager::ensureSDLInited()
{
    if (m_sdlInited) return true;

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
    if (m_windowCount > 0) return;
    if (!m_sdlInited)      return;

    SDL_Quit();
    m_sdlInited = false;
    spdlog::info("GraphSDLManager: SDL остановлен (нет открытых окон)");
    emit allWindowsClosed();
}
