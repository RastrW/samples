#ifndef FORMEXPORTCSV_H
#define FORMEXPORTCSV_H
#pragma once

#include <QDialog>
#include "rmodel.h"

namespace Ui {
class formexportcsv;
}

class formexportcsv : public QDialog
{
    Q_OBJECT

public:
    explicit formexportcsv(RData* _prdata, QWidget *parent = nullptr);
    ~formexportcsv();
private slots:
    void on_pushButton_clicked();
    void accept();

    void on_formexportcsv_accepted();

private:
    Ui::formexportcsv *ui;
    RData* prdata_;
    std::string File;
};

#endif // FORMEXPORTCSV_H
