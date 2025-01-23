#ifndef FORMIMPORTCSV2_H
#define FORMIMPORTCSV2_H

#include <QDialog>
#include "rmodel.h"

namespace Ui {
class formimportcsv2;
}

class formimportcsv2 : public QDialog
{
    Q_OBJECT

public:
    explicit formimportcsv2(RData* _prdata, QWidget *parent = nullptr);
    ~formimportcsv2();

private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();

private:
    Ui::formimportcsv2 *ui;
    RData* prdata_;
};

#endif // FORMIMPORTCSV2_H
