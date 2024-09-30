#include "formsettingsonloadfiles.h"
#include "ui_formsettingsonloadfiles.h"

#include <QFileDialog>


/*
    QString qstrPathToFile = QFileDialog::getOpenFileName(this, tr("Open file"));
    if(qstrPathToFile.length()<3){
        return;
    }
    shEdit_->setFileInfo(QFileInfo{qstrPathToFile});
*/

FormSettingsOnLoadFiles::FormSettingsOnLoadFiles(QWidget *parent)
    :QWidget{parent}
    ,ui(new Ui::FormSettingsOnLoadFiles){
    ui->setupUi(this);
}
FormSettingsOnLoadFiles::~FormSettingsOnLoadFiles(){
    delete ui;
}
