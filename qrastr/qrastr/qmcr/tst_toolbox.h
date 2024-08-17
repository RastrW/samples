#ifndef TST_TOOLBOX_H
#define TST_TOOLBOX_H

//#include <QToolBox>
#include <QDialog>

namespace Ui {
class Tst_ToolBox;
}

class Tst_ToolBox
        //:public QToolBox
        :public QDialog
{
    Q_OBJECT

public:
    explicit Tst_ToolBox(QWidget *parent = nullptr);
    ~Tst_ToolBox();

private:
    Ui::Tst_ToolBox *ui;
};

#endif // TST_TOOLBOX_H
