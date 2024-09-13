#include "ComboBoxDelegate.h"

#include <QComboBox>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QString>
#include <utils.h>

#include <iostream>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    :QItemDelegate(parent)
{
    Items.push_back("Test0");
    Items.push_back("Test1");
    Items.push_back("Test2");
    Items.push_back("Test3");
    Items.push_back("Test4");
    Items.push_back("Test5");
    Items.push_back("Test6");
    Items.push_back("Test7");
    Items.push_back("Test8");
}
ComboBoxDelegate::ComboBoxDelegate(QObject *parent, std::string strItems)
    :QItemDelegate(parent)
{
    for (auto val : split(strItems,','))
    {
        Items.push_back(val) ;
    }
}


QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QComboBox* editor = new QComboBox(parent);
    for(unsigned int i = 0; i < Items.size(); ++i)
    {
        editor->addItem(Items[i].c_str());
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentIndex(value);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    if (index.row() < Items.size() )
    {
        QString text = Items[index.row()].c_str();
        myOption.text = text;
    }
    else
    {
        QString text = Items[Items.size()-1].c_str();
        myOption.text = text;
        //myOption.text =  index.data();
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}
