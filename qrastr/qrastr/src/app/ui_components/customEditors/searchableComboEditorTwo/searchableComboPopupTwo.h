#pragma once
#include <QFrame>

class QLineEdit;
class QTableView;
class QStandardItemModel;
class TwoColumnFilterProxy;
class QPushButton;

// SearchableComboPopupTwo — всплывающий виджет: таблица 2 столбца + фильтры
class SearchableComboPopupTwo : public QFrame
{
    Q_OBJECT
public:
    explicit SearchableComboPopupTwo(QWidget* parent = nullptr);

    /// Заполнить список (key → отображаемое имя)
    void setItems(const std::map<size_t, std::string>& items);
    /// Выделить строку с заданным ключом и очистить фильтры
    void setCurrentKey(int key);

signals:
    void itemSelected   (int key);  ///< пользователь выбрал строку
    void clearSelected  ();         ///< пользователь нажал "Не задано"
    void editingCancelled();        ///< пользователь нажал "Закрыть" или Escape

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void onRowActivated(const QModelIndex& proxyIdx);

    QLineEdit*            m_searchIndex;
    QLineEdit*            m_searchName;
    QTableView*           m_table;
    QStandardItemModel*   m_model;
    TwoColumnFilterProxy* m_proxy;
    QPushButton*          m_btnClose;
    QPushButton*          m_btnNotSet;
};