#ifndef DLGFINDREPL_H
#define DLGFINDREPL_H
#pragma once

#include <QDialog>
#include "../scihlp.h"
namespace Ui {
class DlgFindRepl;
}
class QLineEdit;

class DlgFindRepl : public QDialog
{
    Q_OBJECT
public:
    explicit DlgFindRepl(QWidget* parent = nullptr);

signals:
    void sig_find(SciHlp::FindParams);

private slots:
    void slot_findClicked();

private:
    QLineEdit*   m_leFind  {nullptr};
    QPushButton* m_pbFind  {nullptr};
};
#endif // DLGFINDREPL_H
