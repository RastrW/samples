#pragma once

#include <QObject>


class QAction;
class QMenu;
class QToolBar;
class QStatusBar;
class QMainWindow;

struct RecentFileEntry;

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
    std::map<QString, QAction*> m_actions;
    std::map<QString, QMenu*> m_menus;
    std::map<QString, QToolBar*> m_toolBars;

    // Методы создания
    void createFileActions();
    void createCalcActions();
    void createTIActions();
    void createBarsMDPActions();
    void createGraphActions();
    void createMacroActions();
    void createWindowActions();
    void createHelpActions();
    void createStyleActions();

    QAction* addAction(
        const QString& name,
        const QString& text,
        const QString& iconPath = "",
        const QString& shortcut = "",
        const QString& statusTip = "");
};
