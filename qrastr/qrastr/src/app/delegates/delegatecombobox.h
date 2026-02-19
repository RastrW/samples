#ifndef DELEGATECOMBOBOX_H
#define DELEGATECOMBOBOX_H
#pragma once

#include <string>
#include <vector>

#include <QItemDelegate>
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

class DelegateComboBox : public QItemDelegate
{
    Q_OBJECT
public:
    DelegateComboBox(QObject *parent = 0);
    DelegateComboBox(QObject *parent = 0,std::string strItems = "");


    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    std::vector<std::string> Items;

};


#endif
