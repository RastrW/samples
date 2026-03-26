#include "workspaceManager.h"
#include "FormManager.h"
#include "workspaceSettings/saveWorkspaceDialog.h"
#include "workspaceSettings/loadWorkspaceDialog.h"

#include <QMainWindow>
#include <QSettings>
#include <DockManager.h>
#include <spdlog/spdlog.h>

WorkspaceManager::WorkspaceManager(
    QSettings*          settings,
    ads::CDockManager*  dockManager,
    FormManager*        formManager,
    QMainWindow*        mainWindow,
    QObject*            parent)
    : QObject(parent)
    , m_settings(settings)
    , m_dockManager(dockManager)
    , m_formManager(formManager)
    , m_mainWindow(mainWindow)
{}

QStringList WorkspaceManager::names() const {
    return m_settings->value(kNames).toStringList();
}

QString WorkspaceManager::startupWorkspace() const {
    return m_settings->value(kStartup).toString();
}

void WorkspaceManager::applyStartupWorkspace() {
    const QString startup = startupWorkspace();
    if (startup.isEmpty()) return;

    auto entry = load(startup);
    if (!entry) {
        spdlog::warn("Startup workspace '{}' not found in settings",
                     startup.toStdString());
        return;
    }
    apply(*entry);
}

void WorkspaceManager::slot_showSaveDialog() {
    SaveWorkspaceDialog dlg(names(), startupWorkspace(), m_mainWindow);
    if (dlg.exec() != QDialog::Accepted)
        return;

    // 1. Удаления
    for (const QString& name : dlg.deletedWorkspaceNames())
        remove(name);

    // 2. Флаг «при старте»
    const QString startupName = dlg.startupWorkspaceName();
    m_settings->setValue(kStartup, startupName);
    m_settings->sync();

    // 3. Новая область (имя может быть пустым — пользователь не добавлял)
    const QString newName = dlg.newWorkspaceName();
    if (newName.isEmpty())
        return;

    WorkspaceEntry entry;
    entry.name      = newName;
    entry.adsState  = m_dockManager->saveState();
    entry.openForms = m_formManager->openWidgetNames(); // все доки, включая неактивные вкладки
    save(entry);
    m_settings->sync();
}

void WorkspaceManager::slot_showLoadDialog() {
    LoadWorkspaceDialog dlg(names(), startupWorkspace(), m_mainWindow);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString name = dlg.selectedWorkspace();
    if (name.isEmpty()) return;

    auto entry = load(name);
    if (!entry) {
        spdlog::error("Workspace '{}' not found", name.toStdString());
        return;
    }
    apply(*entry);
}

void WorkspaceManager::save(const WorkspaceEntry& entry) {
    // Обновляем список имён
    QStringList list = names();
    if (!list.contains(entry.name))
        list.append(entry.name);
    m_settings->setValue(kNames, list);
    // Сохраняем данные области
    const QString prefix = QString("%1/%2/").arg(kGroup, entry.name);
    m_settings->setValue(prefix + "adsState",  entry.adsState);
    m_settings->setValue(prefix + "openForms", entry.openForms);
    // геометрию главного окна сохраняем не здесь
}

void WorkspaceManager::remove(const QString& name) {
    QStringList list = names();
    list.removeAll(name);
    m_settings->setValue(kNames, list);
    // Удаляем все ключи секции этой области
    m_settings->remove(QString("%1/%2").arg(kGroup, name));
    // Сбрасываем флаг старта, если удаляли startup-область
    if (startupWorkspace() == name)
        m_settings->setValue(kStartup, QString{});

    spdlog::info("Workspace '{}' removed", name.toStdString());
}

std::optional<WorkspaceEntry> WorkspaceManager::load(const QString& name) const {
    if (!names().contains(name))
        return std::nullopt;

    const QString prefix = QString("%1/%2/").arg(kGroup, name);
    WorkspaceEntry entry;
    entry.name      = name;
    entry.adsState  = m_settings->value(prefix + "adsState").toByteArray();
    entry.openForms = m_settings->value(prefix + "openForms").toStringList();
    return entry;
}

void WorkspaceManager::apply(const WorkspaceEntry& entry) {
    spdlog::info("Applying workspace '{}'", entry.name.toStdString());

    // ── 1. Разбить нужные виджеты ─────────────────────────────────────────
    QSet<QString> neededRegular;
    QSet<QString> neededProtocol;
    for (const QString& name : entry.openForms) {
        if (m_formManager->protocolDockNames().contains(name))
            neededProtocol.insert(name);
        else
            neededRegular.insert(name);
    }

    // Снимок всех доков, зарегистрированных в ADS
    const QMap<QString, ads::CDockWidget*> allDocks = m_dockManager->dockWidgetsMap();

    // ── 2. Текущие открытые НЕпротокольные доки ───────────────────────────
    QSet<QString> currentOpen;
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        const ads::CDockWidget* dw = it.value();
        if (dw && !dw->isClosed() && !m_formManager->protocolDockNames().contains(dw->objectName()))
            currentOpen.insert(dw->objectName());
    }

    // ── 3. Закрыть лишние ─────────────────────────────────────────────────
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        ads::CDockWidget* dw = it.value();
        if (!dw) continue;
        const QString name = dw->objectName();
        if (m_formManager->protocolDockNames().contains(name))      continue; // протокол — не трогаем
        if (dw->isClosed())                continue; // уже закрыт
        if (neededRegular.contains(name))  continue; // нужен — оставляем

        dw->closeDockWidget();
    }

    // ── 4. Открыть недостающие ────────────────────────────────────────────
    for (const QString& name : std::as_const(neededRegular)) {
        if (!currentOpen.contains(name)) {
            m_formManager->openWidgetByName(name);
        }
    }

    // ── 5. Протоколы: только скрыть / показать ────────────────────────────
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        ads::CDockWidget* dw = it.value();
        if (!dw || !m_formManager->protocolDockNames().contains(dw->objectName())) continue;
        dw->toggleView(neededProtocol.contains(dw->objectName()));
    }

    // ── 6. ADS восстанавливает расположение ───────────────────────────────
    if (!entry.adsState.isEmpty())
        m_dockManager->restoreState(entry.adsState);
}