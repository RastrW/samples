#ifndef FORMSETTINGSONLOADTEMPLATES_H
#define FORMSETTINGSONLOADTEMPLATES_H

#include <QDialog>

namespace Ui {
class FormSettingsOnLoadTemplates;
}

class FormSettingsOnLoadTemplates
    : public QDialog{
    Q_OBJECT
public:
    explicit FormSettingsOnLoadTemplates(QWidget *parent = nullptr);
    ~FormSettingsOnLoadTemplates();
    void showEvent( QShowEvent* event )override;
    static constexpr const int n_colnum_checked_      = 0;
    static constexpr const int n_colnum_templatename_ = 1;
private slots:
    void on_pbApply_clicked();
private:
    Ui::FormSettingsOnLoadTemplates *ui;
};

#endif // FORMSETTINGSONLOADTEMPLATES_H
