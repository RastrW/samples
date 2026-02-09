#ifndef FORMSETTINGSONLOADFILES_H
#define FORMSETTINGSONLOADFILES_H
#pragma once

#include <QWidget>
#include "formsettingsstackeditem.h"

namespace Ui {
class FormSettingsOnLoadFiles;
}

class FormSettingsOnLoadFiles
    : public QWidget
    , public FormSettingsStackedItem
{
    Q_OBJECT
public:
    explicit FormSettingsOnLoadFiles(QWidget *parent = nullptr);
    ~FormSettingsOnLoadFiles();
    void showEvent( QShowEvent* event )override;
signals:
private slots:
    void onActTrigNewPathToFile();
    void onChangeData();
private:

    Ui::FormSettingsOnLoadFiles *ui;
};

#endif // FORMSETTINGSONLOADFILES_H
