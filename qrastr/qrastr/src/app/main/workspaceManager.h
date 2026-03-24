#pragma once
#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <optional>

class QSettings;
class QMainWindow;
namespace ads { class CDockManager; }
class FormManager;

/// Запись об одной сохранённой рабочей области
struct WorkspaceEntry {
    QString     name;
    QByteArray  adsState;   ///< m_dockManager->saveState()
    QByteArray  geometry;   ///< mainWindow->saveGeometry()
    QStringList openForms;  ///< objectName() каждого открытого DockWidget
};

/**
 * @brief Менеджер рабочих областей.
 *
 * Отвечает за:
 *  - хранение областей в QSettings (секция "Workspaces/")
 *  - показ SaveWorkspaceDialog / LoadWorkspaceDialog
 *  - применение области (закрыть всё → открыть виджеты → restoreState)
 *  - автозагрузку startup-области при старте
 */
class WorkspaceManager : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceManager(
        QSettings*          settings,
        ads::CDockManager*  dockManager,
        FormManager*        formManager,
        QMainWindow*        mainWindow,
        QObject*            parent = nullptr);

    /// Список имён всех сохранённых областей
    QStringList names() const;

    /// Имя области для загрузки при старте (пусто = нет)
    QString startupWorkspace() const;

    /**
     * @brief Применить startup-область, если она задана.
     * Вызывать ПОСЛЕ того, как все dock-виджеты (формы, лог) созданы.
     */
    void applyStartupWorkspace();

public slots:
    void slot_showSaveDialog();
    void slot_showLoadDialog();

private:
    void save(const WorkspaceEntry& entry, bool loadOnStartup);
    void remove(const QString& name);
    std::optional<WorkspaceEntry> load(const QString& name) const;
    void apply(const WorkspaceEntry& entry);

    QSettings*          m_settings;
    ads::CDockManager*  m_dockManager;
    FormManager*        m_formManager;
    QMainWindow*        m_mainWindow;

    // Ключи QSettings
    static constexpr const char* kNames   = "Workspaces/names";
    static constexpr const char* kStartup = "Workspaces/startup";
    static constexpr const char* kGroup   = "Workspaces";
};