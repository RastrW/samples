#ifndef FORMSETTINGSONLOADFILES_H
#define FORMSETTINGSONLOADFILES_H

#include <QWidget>
#include "formsettingsstackeditem.h"

namespace Ui {
class FormSettingsOnLoadFiles;
}

//class FormSettings;
class FormSettingsOnLoadFiles
    : public QWidget
    , public FormSettingsStackedItem
{
    Q_OBJECT
public:
    explicit FormSettingsOnLoadFiles(QWidget *parent = nullptr);
    ~FormSettingsOnLoadFiles();
signals:
private slots:
    void onActTrigNewPathToFile();
private:
    Ui::FormSettingsOnLoadFiles *ui;
};

#endif // FORMSETTINGSONLOADFILES_H
