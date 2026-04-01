#include "formManager.h"

#include <DockWidget.h>
#include <DockManager.h>
#include "qastra.h"
#include "tableDockManager.h"
#include "web/graphWebManager.h"
#include "sdl/graphSDLManager.h"
#include "macroDockManager.h"
#include "qmcr/pyhlp.h"

FormManager::FormManager(
    std::shared_ptr<QAstra> qastra,
    ads::CDockManager*      dockManager,
    LogManager*             logManager,
    QWidget*                parent)
    : QObject(parent)
    , m_qastra(qastra)
    , m_dockManager(dockManager)
    , m_parentWidget(parent)
    , m_logManager(logManager)
{
    assert(m_qastra      != nullptr);
    assert(m_dockManager != nullptr);
    assert(m_logManager  != nullptr);

    m_pPyHlp    = std::make_shared<PyHlp>(*m_qastra->getRastr().get());

    // ── Таблицы ───────────────────────────────────────────────────────────────
    m_tableDockManager = new TableDockManager(
        m_qastra, m_dockManager, m_pPyHlp, m_parentWidget);

    connect(m_tableDockManager, &TableDockManager::windowOpened,
            this,               &FormManager::registerDockWidget);

    // Пробрасываем сигналы таблиц наружу
    connect(m_tableDockManager, &TableDockManager::formOpened,
            this,               &FormManager::formOpened);
    connect(m_tableDockManager, &TableDockManager::formClosed,
            this,               &FormManager::formClosed);
    connect(m_tableDockManager, &TableDockManager::activeFormChanged,
            this,               &FormManager::activeFormChanged);

    // ── Макросы ───────────────────────────────────────────────────────────────
    m_macroDockManager = new MacroDockManager(
        m_dockManager, m_pPyHlp, m_qastra, m_parentWidget);

    connect(m_macroDockManager, &MacroDockManager::windowOpened,
            this,               &FormManager::registerDockWidget);

    // ── Графика ───────────────────────────────────────────────────────────────
    m_graphSDLManager = new GraphSDLManager(
        m_dockManager, m_parentWidget, m_qastra->getRastr().get(), this);
    m_graphWebManager = new GraphWebManager(
        m_dockManager, m_parentWidget, m_qastra->getRastr().get(), this);

    for (IGraphManager* mgr : {m_graphSDLManager, m_graphWebManager}) {
        connect(mgr,  &IGraphManager::windowOpened,
                this, &FormManager::registerDockWidget);
    }

    // ── Протоколы ─────────────────────────────────────────────────────────────
    connect(m_logManager, &LogManager::dockWidgetCreated,
            this,         &FormManager::registerDockWidget);
}

void FormManager::prepareForClose()
{
    // 1. Закрываем все dock-виджеты, кроме протоколов
    //    (они с CustomCloseHandling — просто скрываем)
    const auto allDocks = m_dockManager->dockWidgetsMap();
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        ads::CDockWidget* dw = it.value();
        if (!dw || dw->isClosed()) continue;

        if (m_protocolNames.contains(dw->objectName()))
            dw->toggleView(false);   // скрыть, не удалять
        else
            dw->closeDockWidget();   // удалить (DockWidgetDeleteOnClose)
    }

    // 2. Уничтожаем оставшиеся floating-контейнеры напрямую.
    //    После шага 1 их быть не должно, но на случай
    //    если какой-то виджет проигнорировал closeDockWidget.
    const auto floating = m_dockManager->floatingWidgets();
    for (ads::CFloatingDockContainer* fc : floating) {
        if (fc) fc->deleteLater();
    }

    m_openDockWidgets.clear();
}

// Делегаты — таблицы
void FormManager::setForms(const std::list<CUIForm>& forms){
    m_tableDockManager->setForms(forms);
}

void FormManager::buildFormsMenu(QMenu* parentMenu, QMenu* calcParametersMenu){
    m_tableDockManager->buildFormsMenu(parentMenu, calcParametersMenu);
}

void FormManager::buildPropertiesMenu(QMenu* propertiesMenu){
    m_tableDockManager->buildPropertiesMenu(propertiesMenu);
}

RtabWidget* FormManager::activeForm() const{
    return m_tableDockManager->activeForm();
}
// Делегаты — макросы
void FormManager::openMacroWindow()
{
    m_macroDockManager->openWindow();
}
// Делегаты — графика
void FormManager::slot_openSDLGraph()
{
    m_graphSDLManager->openWindow();
    emit formOpened(QStringLiteral("Графика SDL"));
}

void FormManager::slot_openWebGraph()
{
    m_graphWebManager->openWindow();
    emit formOpened(QStringLiteral("Графика Web"));
}

void FormManager::closeGraphWebServer()
{
    m_graphWebManager->closeAll();
}
// Делегаты — лог
void FormManager::createLogWidgets()
{
    m_logManager->createWidgets();
}

void FormManager::setupLogDockWidgets()
{
    m_logManager->setupDockWidgets();
}

void FormManager::slot_openProtocol()
{
    m_logManager->openProtocol();
}

// Расчёты
void FormManager::slot_calculationStarted()
{
    m_tableDockManager->slot_calculationStarted();
}

void FormManager::slot_calculationFinished()
{
    m_tableDockManager->slot_calculationFinished();
}

// Реестр dock-виджетов
void FormManager::registerDockWidget(ads::CDockWidget* dw)
{
    if (!dw || m_openDockWidgets.contains(dw)) return;

    m_openDockWidgets.append(dw);

    // Удаляем из списка только если виджет реально уничтожается при закрытии.
    // Лог-виджеты имеют CustomCloseHandling (только скрываются) —
    // они остаются в списке навсегда и всегда участвуют в cascade/tile.
    if (dw->features().testFlag(ads::CDockWidget::DockWidgetDeleteOnClose)) {
        connect(dw, &ads::CDockWidget::closed,
                this, [this, dw]() {
                    m_openDockWidgets.removeOne(dw);
                });
    }
}

// Рабочая область
const QSet<QString>& FormManager::protocolDockNames() const
{
    return m_protocolNames;
}

QStringList FormManager::openWidgetNames() const
{
    QStringList result;
    const auto& allDocks = m_dockManager->dockWidgetsMap();
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        const ads::CDockWidget* dw = it.value();
        if (dw && !dw->isClosed() && !dw->objectName().isEmpty())
            result.append(dw->objectName());
    }
    return result;
}

void FormManager::openWidgetByName(const QString& name)
{
    try {
        // ── Протоколы: только показать ────────────────────────────────────────
        if (m_protocolNames.contains(name)) {
            const auto allDocks = m_dockManager->dockWidgetsMap();
            const auto it = allDocks.find(name);
            if (it != allDocks.end()) {
                it.value()->toggleView(true);
            } else {
                spdlog::warn("Log dock '{}' not in ADS yet, skipping",
                             name.toStdString());
            }
            return;
        }

        // ── Макрос ────────────────────────────────────────────────────────────
        if (name == QLatin1String(MacroDockManager::kDockObjectName)) {
            m_macroDockManager->openWindow();
            return;
        }

        // ── Графика ───────────────────────────────────────────────────────────
        if (name == QStringLiteral("Графика SDL")) { slot_openSDLGraph(); return; }
        if (name == QStringLiteral("Графика Web")) { slot_openWebGraph(); return; }

        // ── Таблицы ───────────────────────────────────────────────────────────
        m_tableDockManager->openFormByName(name);

    } catch (const std::exception& ex) {
        spdlog::error("FormManager::openWidgetByName('{}') threw: {}",
                      name.toStdString(), ex.what());
    } catch (...) {
        spdlog::error("FormManager::openWidgetByName('{}') threw unknown exception",
                      name.toStdString());
    }
}

void FormManager::closeAllWidgets()
{
    // Итерируем по снимку карты: closeDockWidget() модифицирует ADS
    const auto allDocks = m_dockManager->dockWidgetsMap();
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        ads::CDockWidget* dw = it.value();
        if (!dw)                                          continue;
        if (m_protocolNames.contains(dw->objectName()))  continue;
        if (!dw->isClosed())
            dw->closeDockWidget();
    }

    // Локальные реестры чистим напрямую — закрытые dock-виджеты уже
    // удалились сами (DockWidgetDeleteOnClose) или только скрылись.
    m_openDockWidgets.clear();
}

// Cascade / Tile
void FormManager::slot_cascadeForms()
{
    if (m_openDockWidgets.isEmpty()) return;

    // Берём геометрию области dock manager как опорную
    const QRect  available = m_dockManager->contentsRect();
    const QPoint origin    = m_dockManager->mapToGlobal(available.topLeft())
                          + QPoint(20, 20);

    constexpr int kOffset = 30; // шаг каскада в пикселях
    constexpr int kWidth  = 600;
    constexpr int kHeight = 400;

    int step = 0;
    for (ads::CDockWidget* dw : m_openDockWidgets) {
        if (!dw) continue;
        // Открепляем виджет — делаем floating
        dw->setFloating();

        const QPoint pos = origin + QPoint(step * kOffset, step * kOffset);
        if (auto* c = dw->dockContainer(); c && c->isFloating())
            c->window()->setGeometry(pos.x(), pos.y(), kWidth, kHeight);

        ++step;
    }
}

void FormManager::slot_tileForms()
{
    if (m_openDockWidgets.isEmpty()) return;

    // contentsRect() исключает тулбар / строку состояния MainWindow
    const QRect  available = m_dockManager->contentsRect(); //m_dockManager->geometry();
    const QPoint origin    = m_dockManager->mapToGlobal(available.topLeft());

    const int count = m_openDockWidgets.size();
    // Вычисляем сетку: cols x rows
    const int cols  = qMax(1, static_cast<int>(std::ceil(std::sqrt(count))));
    const int rows  = (count + cols - 1) / cols;
    const int w     = available.width()  / cols;
    const int h     = available.height() / rows;

    int i = 0;
    for (ads::CDockWidget* dw : m_openDockWidgets) {
        if (!dw) continue;

        dw->setFloating();

        const int    col = i % cols;
        const int    row = i / cols;
        const QPoint pos = origin + QPoint(col * w, row * h);

        if (auto* c = dw->dockContainer(); c && c->isFloating())
            c->window()->setGeometry(pos.x(), pos.y(), w, h);

        ++i;
    }
}