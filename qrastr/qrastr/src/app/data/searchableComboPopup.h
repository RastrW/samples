#pragma once
#include <QFrame>

class QLineEdit;
class QListView;
class QSortFilterProxyModel;
class QStringListModel;

/// Всплывающий виджет: строка поиска + список
class SearchableComboPopup : public QFrame
{
    Q_OBJECT
public:
    explicit SearchableComboPopup(QWidget* parent = nullptr);
    void setItems(const QStringList& items);
    void setCurrentText(const QString& text);
    QString currentText() const;

signals:
    void itemSelected(const QString& value);
    void editingCancelled();

protected:
    //void keyPressEvent(QKeyEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    QLineEdit*            m_search;
    QListView*            m_list;
    QSortFilterProxyModel* m_proxy;
    QStringListModel*     m_model;

    void applyFilter(const QString& text);
};