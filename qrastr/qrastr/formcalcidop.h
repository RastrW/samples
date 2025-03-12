#ifndef FORMCALCIDOP_H
#define FORMCALCIDOP_H

#include <QDialog>
#include "qastra.h"


namespace Ui {
class formcalcidop;
}

class formcalcidop : public QDialog
{
    Q_OBJECT

public:
    explicit formcalcidop(QAstra* pqastra,QWidget *parent = nullptr);
    ~formcalcidop();

private slots:
    void on_buttonBox_accepted();
    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::formcalcidop *ui;
    QAstra* pqastra;

};

#endif // FORMCALCIDOP_H
