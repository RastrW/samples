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
    QStringList openForms;  ///< objectName() каждого открытого DockWidget
    ///< (включая неактивные вкладки в группах)
};

/**
 * @brief Менеджер рабочих областей.
 *
 * Отвечает за:
 *  - хранение областей в QSettings (секция "Workspaces/")
 *  - показ SaveWorkspaceDialog / LoadWorkspaceDialog
 *  - применение области:
 *      закрыть лишние → открыть недостающие →
 *      скрыть/показать протоколы → restoreState ADS
 *  - автозагрузку startup-области при старте
 *
 * Протоколы ("Полный протокол", "Протокол Astra") никогда не закрываются —
 * только скрываются / показываются через toggleView().
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
    // ── Имена протокольных доков (никогда не закрываются) ─────────────────
    static const QSet<QString>& protocolNames();

    // ── Работа с хранилищем ───────────────────────────────────────────────
    void save(const WorkspaceEntry& entry);
    void remove(const QString& name);
    std::optional<WorkspaceEntry> load(const QString& name) const;

    /**
     * @brief Применение области
     *
     * Алгоритм:
     *  1. Разбить нужные виджеты на обычные и протоколы.
     *  2. Собрать текущие открытые НЕпротокольные доки.
     *  3. Закрыть лишние (те, что открыты, но не нужны).
     *  4. Открыть недостающие (те, что нужны, но ещё не открыты).
     *  5. Протоколы: toggleView — никогда closeDockWidget.
     *  6. ADS восстанавливает расположение через restoreState.
     */
    void apply(const WorkspaceEntry& entry);

    // ── Зависимости ───────────────────────────────────────────────────────
    QSettings*          m_settings;
    ads::CDockManager*  m_dockManager;
    FormManager*        m_formManager;
    QMainWindow*        m_mainWindow;

    // ── Ключи QSettings ───────────────────────────────────────────────────
    static constexpr const char* kNames   = "Workspaces/names";
    static constexpr const char* kStartup = "Workspaces/startup";
    static constexpr const char* kGroup   = "Workspaces";
};