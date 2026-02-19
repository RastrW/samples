#ifndef FORMSETTINGSONLOADFORMS_H
#define FORMSETTINGSONLOADFORMS_H
#pragma once

#include <QWidget>

namespace Ui {
class FormSettingsOnLoadForms;
}

class FormSettingsOnLoadForms : public QWidget
{
    Q_OBJECT

public:
    explicit FormSettingsOnLoadForms(QWidget *parent = nullptr);
    ~FormSettingsOnLoadForms();
    void showEvent( QShowEvent* event )override;
private slots:
    void on_pbApply_clicked();
private:
    Ui::FormSettingsOnLoadForms *ui;
    static constexpr const int n_colnum_checked_  = 0;
    static constexpr const int n_colnum_formname_ = 1;
};

#endif // FORMSETTINGSONLOADFORMS_H
