#include "formsettingsonloadfiles.h"
#include "ui_formsettingsonloadfiles.h"

#include <QFileDialog>
#include "params.h"
#include "formsettings.h"

FormSettingsOnLoadFiles::FormSettingsOnLoadFiles(QWidget *parent)
    : QWidget{parent}
    , ui{new Ui::FormSettingsOnLoadFiles}
    , FormSettingsStackedItem{qobject_cast<FormSettings*>(parent)}{
    ui->setupUi(this);
    std::string str = Params::GetInstance()->Get_on_start_load_file_rastr();
    ui->lePathToLoadFile->setText(str.c_str());
    QAction* act_trig_path_to_load_file = ui->lePathToLoadFile->addAction(QIcon(":/images/open.png"), QLineEdit::LeadingPosition); // TrailingPosition
    //bool bl_res = connect( act_trig_path_to_load_file, &QAction::triggered, [this](){  assert(0);  }); assert(bl_res == true);
    bool bl_res = connect( act_trig_path_to_load_file, &QAction::triggered,this, &FormSettingsOnLoadFiles::onActTrigNewPathToFile ); assert(bl_res == true);
}
FormSettingsOnLoadFiles::~FormSettingsOnLoadFiles(){
    delete ui;
}
void FormSettingsOnLoadFiles::onActTrigNewPathToFile(){
    QString qstr_path_to_file = QFileDialog::getOpenFileName(this, tr("Open file"));
    if(qstr_path_to_file.length()<3){
        return;
    }
    ui->lePathToLoadFile->setText(qstr_path_to_file);
    getFormSettings()->setButtonSaveEnabled(true);
}

