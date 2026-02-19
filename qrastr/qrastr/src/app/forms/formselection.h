#ifndef FORMSELECTION_H
#define FORMSELECTION_H
#pragma once

#include <QDialog>

namespace Ui {
class FormSelection;
}

class FormSelection : public QDialog
{
    Q_OBJECT

public:
    explicit FormSelection( std::string _selection ,QWidget *parent = nullptr);
    ~FormSelection();

private slots:
    void on_buttonBox_accepted();

private:
    std::string selection;
    Ui::FormSelection *ui;
};

#endif // FORMSELECTION_H
