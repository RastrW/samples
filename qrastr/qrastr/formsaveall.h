#ifndef FORMSAVEALL_H
#define FORMSAVEALL_H

#include <QDialog>
#include "QAstra.h"
#include <QDir>

class QAstra;

namespace Ui {
class formsaveall;
}

class formsaveall : public QDialog
{
    Q_OBJECT

public:
    explicit formsaveall(QAstra* _pqastra,QMap<QString,QString> _mFilesLoad,QWidget  *parent = nullptr);
    ~formsaveall();
    void showEvent( QShowEvent* event )override;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::formsaveall *ui;
    QAstra* pqastra;
    QDir dir_shabl;
    QMap<QString,QString> mFilesLoad;

    enum class _cols : int  { save , templ , file , path };

};

#endif // FORMSAVEALL_H
