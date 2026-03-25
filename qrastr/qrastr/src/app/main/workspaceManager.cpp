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
    , m_mainWindow(mainWindow){}

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
    SaveWorkspaceDialog dlg(names(), m_mainWindow);
    if (dlg.exec() != QDialog::Accepted)
        return;

    // 1. Удаления
    for (const QString& name : dlg.deletedWorkspaceNames())
        remove(name);

    // 2. Флаг «при старте» — независимо от того, добавляется ли новая область
    const QString startupName = dlg.startupWorkspaceName();
    m_settings->setValue(kStartup, startupName); // пусто = сброс
    m_settings->sync();

    // 3. Новая область — только если имя введено
    const QString newName = dlg.newWorkspaceName();
    if (newName.isEmpty())
        return;

    WorkspaceEntry entry;
    entry.name      = newName;
    entry.adsState  = m_dockManager->saveState();
    entry.geometry  = m_mainWindow->saveGeometry();
    entry.openForms = m_formManager->openWidgetNames();
    save(entry);
    m_settings->sync();
}

void WorkspaceManager::slot_showLoadDialog() {
    LoadWorkspaceDialog dlg(names(), m_mainWindow);
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
    m_settings->setValue(prefix + "geometry",  entry.geometry);
    m_settings->setValue(prefix + "openForms", entry.openForms);
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
    entry.geometry  = m_settings->value(prefix + "geometry").toByteArray();
    entry.openForms = m_settings->value(prefix + "openForms").toStringList();
    return entry;
}

void WorkspaceManager::apply(const WorkspaceEntry& entry) {
    spdlog::info("Applying workspace '{}'", entry.name.toStdString());

    // 1. Закрыть все виджеты включая протокол
    m_formManager->closeAllWidgets();

    // 2. Восстановить геометрию главного окна
    if (!entry.geometry.isEmpty())
        m_mainWindow->restoreGeometry(entry.geometry);

    // 3. Создать виджеты по сохранённым именам
    //    (ADS узнает их при restoreState и разместит по сохранённой геометрии)
    for (const QString& widgetName : entry.openForms)
        m_formManager->openWidgetByName(widgetName);

    // 4. Восстановить расположение в ADS
    if (!entry.adsState.isEmpty())
        m_dockManager->restoreState(entry.adsState);
}