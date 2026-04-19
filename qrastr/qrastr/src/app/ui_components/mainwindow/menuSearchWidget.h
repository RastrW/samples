#pragma once

#include <QWidget>

class QListWidgetItem;
class QLineEdit;
class QListWidget;


/// @class Виджет поиска по пунктам меню
/// Добавляется на тулбар. При вводе текста показывает popup
/// со списком подходящих QAction, клик/Enter — вызывает action->trigger().
class MenuSearchWidget : public QWidget {
    Q_OBJECT
public:
    explicit MenuSearchWidget(QWidget* parent = nullptr);

    /// Добавить действия из карты
    void setActions(const std::map<QString, QAction*>& actions);

    /// Рекурсивно добавить действия из меню и всех подменю.
    void addActionsFromMenu(QMenu* menu);
private slots:
    void onTextChanged(const QString& text);
    void onItemActivated(QListWidgetItem* item);
    void onReturnPressed();

private:
    void showPopup();
    void hidePopup();
    void repositionPopup();
    void registerAction(QAction* action);

    QLineEdit*   m_edit  = nullptr;
    QListWidget* m_popup = nullptr;

    QMap<QString, QAction*> m_actionMap; // отображаемый текст → QAction*
};