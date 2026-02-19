#include "delegatecheckbox.h"
#include <QApplication>

DelegateCheckBox::DelegateCheckBox(QObject *parent)
    : QItemDelegate{parent}
{}

QWidget *DelegateCheckBox::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid() )
    {
        QCheckBox *editor = new QCheckBox(parent);
        editor->installEventFilter(const_cast<DelegateCheckBox*>(this));
        return editor;
    }
    else
    {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

void DelegateCheckBox::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    if(index.isValid())
    {
       // QString value = index.model()->data(index, Qt::DisplayRole).toString();
        int value = index.model()->data(index, Qt::EditRole).toUInt();

        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        if(value > 0)
            checkBox->setCheckState(Qt::Checked);
        else
            checkBox->setCheckState(Qt::Unchecked);
    }
    else
    {
        QItemDelegate::setEditorData(editor, index);
    }
}

void DelegateCheckBox::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    if(index.isValid())
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        //QString value;
        int value;
        if(checkBox->checkState() == Qt::Checked)
            value = 1;
        else
            value = 0;

        model->setData(index, value);
    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

void DelegateCheckBox::updateEditorGeometry(QWidget *editor,
                                            const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid() )
        editor->setGeometry(option.rect);
    else
        QItemDelegate::updateEditorGeometry(editor, option, index);

}
void DelegateCheckBox::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    if (index.data().toInt() > 0)
    {
        myOption.checkState = Qt::Checked;
        myOption.text = "true";
    }
    else
    {
        myOption.checkState = Qt::Unchecked;
        myOption.text = "false";
    }

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &myOption, painter);

    //drawDisplay(painter,myOption,myOption.rect,index.model()->data( index, Qt::DisplayRole ).toBool()?QString("      ").append(tr("Var")):QString("      ").append(tr("Yok")));
    drawDisplay(painter,myOption,myOption.rect,index.model()->data( index, Qt::DisplayRole ).toBool()?QString("").append(tr("true")):QString("").append(tr("false")));
    drawFocus(painter,myOption,myOption.rect);
}
