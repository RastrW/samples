#include "doubleitemdelegate.h"

QString DoubleItemDelegate::displayText(const QVariant & value, const QLocale & locale) const
{
    QString str = QString::number(value.toDouble(), 'f', m_digits);
    return str;
}
