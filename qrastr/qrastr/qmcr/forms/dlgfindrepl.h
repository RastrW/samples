#ifndef DLGFINDREPL_H
#define DLGFINDREPL_H

#include <QDialog>
#include "../scihlp.h"
namespace Ui {
class DlgFindRepl;
}
//struct SciHlp::_params_findrepl;
class DlgFindRepl : public QDialog
{
    Q_OBJECT

public:
    explicit DlgFindRepl(QWidget *parent = nullptr);
    ~DlgFindRepl();
signals:
    void chngFindRepl(SciHlp::_params_findrepl);
private slots:
    void on_pbFind_clicked();

private:
    Ui::DlgFindRepl *ui;
};

#endif // DLGFINDREPL_H
