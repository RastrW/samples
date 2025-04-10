#ifndef FORMGROUPCORRECTION_H
#define FORMGROUPCORRECTION_H
#pragma once

#include <QDialog>
#include "rmodel.h"

namespace Ui {
class formgroupcorrection;
}

class formgroupcorrection : public QDialog
{
    Q_OBJECT

public:
    explicit formgroupcorrection(RData* _prdata,RCol* prcol,QWidget *parent = nullptr);
    ~formgroupcorrection();

private slots:
    void on_buttonBox_accepted();

private:
    RData* prdata_;
    RCol* prcol_;
    std::string selection_;
    std::string expression_;
    Ui::formgroupcorrection *ui;
};

#endif // FORMGROUPCORRECTION_H
