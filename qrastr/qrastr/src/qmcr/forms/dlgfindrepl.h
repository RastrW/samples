#pragma once

#include <QDialog>
#include "../sciPyEditor.h"

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
    void sig_find(SciPyEditor::FindParams);

private slots:
    void slot_findClicked();

private:
    QLineEdit*   m_leFind  {nullptr};
    QPushButton* m_pbFind  {nullptr};
};
