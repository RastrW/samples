#pragma once

#include <QObject>
#include <QMainWindow>
#include <QString>
#include <QMap>

class QAction;
class QMenu;
class QToolBar;
class QStatusBar;

/// @class Построитель UI элементов
class UIBuilder : public QObject {
    Q_OBJECT

public:
    explicit UIBuilder(QMainWindow* mainWindow);
    ~UIBuilder() = default;

    // ========== Построение UI ==========
    /// @brief Построить все UI элементы
    void buildAll();
    // ========== Доступ к элементам ==========
    QAction* actionByName(const QString& name) const;
    QMenu* menuByName(const QString& name) const;
    QToolBar* toolBarByName(const QString& name) const;

    // Прямой доступ к часто используемым меню
    QMenu* fileMenu() const { return m_menus.value("file"); }
    QMenu* openMenu() const { return m_menus.value("open"); }
    QMenu* calcMenu() const { return m_menus.value("calc"); }
    QMenu* calcParametersMenu() const { return m_menus.value("calcParameters"); }
    QMenu* propertiesMenu() const { return m_menus.value("properties"); }
    QMenu* recentFilesMenu() const { return m_menus.value("recentFiles"); }

    // ========== Обновление недавних файлов ==========
    /**
     * @brief Обновить действия недавних файлов
     * @param recentFiles список строк вида "file <template>"
     */
    void updateRecentFileActions(const QStringList& recentFiles);
private:
    /// @brief Построить только меню
    void buildMenuBar();
    /// @brief Построить панели инструментов
    void buildToolBars();
    /// @brief Построить статусную строку
    void buildStatusBar();
signals:
    void actionTriggered(const QString& actionName);

private:
    QMainWindow* m_mainWindow;

    // Хранилища UI элементов
    QMap<QString, QAction*> m_actions;
    QMap<QString, QMenu*> m_menus;
    QMap<QString, QToolBar*> m_toolBars;

    // Константы
    static constexpr int kMaxRecentFiles = 10;

    // Методы создания
    void createFileActions();
    void createCalcActions();
    void createTIActions();
    void createBarsMDPActions();
    void createGraphActions();
    void createMacroActions();
    void createWindowActions();
    void createHelpActions();

    QAction* addAction(
        const QString& name,
        const QString& text,
        const QString& iconPath = "",
        const QString& shortcut = "",
        const QString& statusTip = "");
};
