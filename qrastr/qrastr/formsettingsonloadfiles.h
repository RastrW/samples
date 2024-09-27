#ifndef FORMSETTINGSONLOADFILES_H
#define FORMSETTINGSONLOADFILES_H

#include <QWidget>


namespace Ui {
class FormSettingsOnLoadFiles;
}

class FormSettingsOnLoadFiles
    : public QWidget
{
    Q_OBJECT
public:
    explicit FormSettingsOnLoadFiles(QWidget *parent = nullptr);
    ~FormSettingsOnLoadFiles();
signals:
private:
    Ui::FormSettingsOnLoadFiles *ui;

};

#endif // FORMSETTINGSONLOADFILES_H
