#ifndef FORMSETTINGSDATAS_H
#define FORMSETTINGSDATAS_H
#pragma once

#include <QWidget>

namespace Ui {
class FormSettingsDatas;
}

class FormSettingsDatas : public QWidget
{
    Q_OBJECT

public:
    explicit FormSettingsDatas(QWidget *parent = nullptr);
    ~FormSettingsDatas();

private:
    Ui::FormSettingsDatas *ui;
};

#endif // FORMSETTINGSDATAS_H
