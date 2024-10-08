#ifndef DELEGATECHECKBOX_H
#define DELEGATECHECKBOX_H

#include <QItemDelegate>
#include <QObject>
#include <QCheckBox>
#include <QStyledItemDelegate>

class DelegateCheckBox : public QItemDelegate
{
    Q_OBJECT
public:
   // explicit DelegateCheckBox(QObject *parent = nullptr);
    DelegateCheckBox(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor,  const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DELEGATECHECKBOX_H
