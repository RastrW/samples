#ifndef FORMSETTINGSFORMS_H
#define FORMSETTINGSFORMS_H
#pragma once

#include <QWidget>

namespace Ui {
class FormSettingsForms;
}

class FormSettingsForms : public QWidget
{
    Q_OBJECT

public:
    explicit FormSettingsForms(QWidget *parent = nullptr);
    ~FormSettingsForms();

private:
    Ui::FormSettingsForms *ui;
};

#endif // FORMSETTINGSFORMS_H
