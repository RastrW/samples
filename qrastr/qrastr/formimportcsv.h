#ifndef FORMIMPORTCSV_H
#define FORMIMPORTCSV_H

#include <QDialog>
#include "rmodel.h"

namespace Ui {
class formimportcsv;
}

class formimportcsv : public QDialog
{
    Q_OBJECT

public:
    explicit formimportcsv(RData* _prdata, QWidget *parent = nullptr);
    ~formimportcsv();
private slots:
    //void on_pushButton_file_clicked();
    //void on_buttonBox_accepted();
    //void on_pushButton_file_pressed();
    void accept();

   /* void on_pushButton_clicked();

    void on_pushButton_f_clicked();*/

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::formimportcsv *ui;
    RData* prdata_;
    std::string File;
};

#endif // FORMIMPORTCSV_H
