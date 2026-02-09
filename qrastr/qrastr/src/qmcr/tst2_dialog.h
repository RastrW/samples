#ifndef TST2_DIALOG_H
#define TST2_DIALOG_H

#include <QDialog>

namespace Ui {
class Tst2_Dialog;
}

class Tst2_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Tst2_Dialog(QWidget *parent = nullptr);
    ~Tst2_Dialog();

private:
    Ui::Tst2_Dialog *ui;
};

#endif // TST2_DIALOG_H
