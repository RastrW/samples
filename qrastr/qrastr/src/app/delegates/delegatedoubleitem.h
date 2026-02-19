#ifndef DELEGATEDOUBLEITEM_H
#define DELEGATEDOUBLEITEM_H
#pragma once

#include <QObject>
#include <QStyledItemDelegate>

class DelegateDoubleItem : public QStyledItemDelegate
{

    Q_OBJECT

public:

    DelegateDoubleItem(const int digits = 1, QObject *parent = 0) : QStyledItemDelegate(parent) {m_digits = digits; }
    QString displayText(const QVariant & value, const QLocale & locale) const;
    void set_prec(int prec);
private:
    int m_digits;
};

#endif // DELEGATEDOUBLEITEM_H
