#include "macroDockManager.h"
#include <DockManager.h>
#include "mcrwnd.h"
#include "log/ILogEvents.h"

MacroDockManager::MacroDockManager(
    ads::CDockManager*      dockManager,
    std::shared_ptr<PyHlp>  pyHlp,
    std::shared_ptr<ILogEvents> logEvents,
    QWidget*                parent)
    : QObject(parent)
    , m_dockManager(dockManager)
    , m_pyHlp(pyHlp)
    , m_logEvents(logEvents)
    , m_parentWidget(parent)
{}

void MacroDockManager::openWindow()
{
    // ── Dock уже создан — просто показываем / поднимаем ──────────────────────
    if (m_dockWidget) {
        m_dockWidget->toggleView(true);
        m_dockWidget->setAsCurrentTab();
        return;
    }

    // ── Первый вызов: создаём McrWnd и оборачиваем в CDockWidget ─────────────
    m_mcrWnd = new McrWnd(m_parentWidget);
    m_mcrWnd->setPyHlp(m_pyHlp);

    auto* dw = new ads::CDockWidget(
        tr("Macro Editor — Python"), m_parentWidget);
    dw->setObjectName(QLatin1String(kDockObjectName));

    // Не удалять при закрытии — только скрывать (поведение как у протоколов).
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, false);
    // Перехватываем закрытие сами (диалог сохранения).
    dw->setFeature(ads::CDockWidget::CustomCloseHandling,     true);

    dw->setWidget(m_mcrWnd);

    // Синхронизируем заголовок вкладки с именем файла
    connect(m_mcrWnd, &McrWnd::titleChanged,
            dw,       &ads::CDockWidget::setWindowTitle);

    // Перехват closeRequested: показываем диалог, при отмене — не закрываем
    connect(dw, &ads::CDockWidget::closeRequested,
            this, [this, dw]() {
                if (m_mcrWnd && m_mcrWnd->promptAndAllowClose())
                    dw->toggleView(false);
            });

    // Подключаем вывод rastr-сообщений в лог редактора
    connect(m_logEvents.get(), &ILogEvents::sig_rastrPrint,
            m_mcrWnd,       &McrWnd::slot_rastrPrint);

    m_dockWidget = dw;
    m_dockManager->addDockWidget(ads::RightDockWidgetArea, dw);

    emit windowOpened(dw);
}

bool MacroDockManager::isOpen() const
{
    return m_dockWidget && !m_dockWidget->isClosed();
}