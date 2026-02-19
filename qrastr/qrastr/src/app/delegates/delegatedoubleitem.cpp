#include "delegatedoubleitem.h"

QString DelegateDoubleItem::displayText(const QVariant & value, const QLocale & locale) const
{
    QString str = QString::number(value.toDouble(), 'f', m_digits);
    return str;
}

void DelegateDoubleItem::set_prec(int prec)
{
    m_digits = prec;
}
