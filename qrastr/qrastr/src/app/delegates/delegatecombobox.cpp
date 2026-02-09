#include "delegatecombobox.h"

#include <QComboBox>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QString>
#include <utils.h>

#include <iostream>

DelegateComboBox::DelegateComboBox(QObject *parent)
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
DelegateComboBox::DelegateComboBox(QObject *parent, std::string strItems)
    :QItemDelegate(parent)
{
    char delimiter = '|';
    if (strItems.find_first_of(',') != std::variant_npos)
        delimiter = ',';

    for (auto val : split(strItems,delimiter))
    {
        Items.push_back(val) ;
    }
}


QWidget *DelegateComboBox::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    QComboBox* editor = new QComboBox(parent);
    for(unsigned int i = 0; i < Items.size(); ++i)
    {
        editor->addItem(Items[i].c_str());
    }
    return editor;
}

void DelegateComboBox::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentIndex(value);
}

void DelegateComboBox::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}

void DelegateComboBox::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void DelegateComboBox::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;

    if (index.data().toInt() < Items.size() )
    {
        //QString text = Items[index.row()].c_str();
        QString text = Items[ (index.data().toInt())].c_str();
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
