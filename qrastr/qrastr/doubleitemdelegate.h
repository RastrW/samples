#ifndef DOUBLEITEMDELEGATE_H
#define DOUBLEITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class DoubleItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

public:

    DoubleItemDelegate(const int digits = 1, QObject *parent = 0) : QStyledItemDelegate(parent) {m_digits = digits; }
    QString displayText(const QVariant & value, const QLocale & locale) const;
    void set_prec(int prec);
private:
    int m_digits;
};

#endif // DOUBLEITEMDELEGATE_H
