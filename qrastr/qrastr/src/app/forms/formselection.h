#pragma once

#include <QDialog>

namespace Ui {
class FormSelection;
}

class FormSelection : public QDialog
{
    Q_OBJECT

public:
    explicit FormSelection(std::string colName,
                           QWidget *parent = nullptr);
    ~FormSelection();
signals:
    void sig_selectionAccepted(std::string selection);
private slots:
    void on_buttonBox_accepted();

private:
    std::string m_colName;
    Ui::FormSelection *ui;
};
