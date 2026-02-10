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
    ui->cbTemplate->clear();
    for(const auto& str_template_ext : Params::GetInstance()->getTemplateExts()){
        const QVariant qv_user_data_null;
        QString qstr = QString("%1%2").arg(str_template_ext.first.c_str()).arg(str_template_ext.second.c_str());
        ui->cbTemplate->addItem(qstr, qv_user_data_null);
    }
    if(Params::GetInstance()->getStartLoadFileTemplates().size()>0){
        ui->lePathToLoadFile->setText ( Params::GetInstance()->getStartLoadFileTemplates()[0].first .c_str() );
        ui->cbTemplate->setCurrentText( Params::GetInstance()->getStartLoadFileTemplates()[0].second.c_str() ); //int n_item = ui->cbTemplate->findText(qstr_templ);
    }
    QAction* act_trig_path_to_load_file = ui->lePathToLoadFile->addAction(QIcon(":/images/open.png"), QLineEdit::LeadingPosition); // TrailingPosition
    bool bl_res = connect(act_trig_path_to_load_file, &QAction::triggered, this, &FormSettingsOnLoadFiles::onActTrigNewPathToFile);
    assert(bl_res == true);
    bl_res = connect(ui->lePathToLoadFile, &QLineEdit::editingFinished, this, &FormSettingsOnLoadFiles::onChangeData);
    assert(bl_res == true);
    bl_res = connect(ui->cbTemplate, &QComboBox::currentTextChanged, this, &FormSettingsOnLoadFiles::onChangeData);
    assert(bl_res == true);
}

FormSettingsOnLoadFiles::~FormSettingsOnLoadFiles(){
    delete ui;
}
void FormSettingsOnLoadFiles::showEvent( QShowEvent* event ){
}
void FormSettingsOnLoadFiles::onActTrigNewPathToFile(){
    QString qstr_path_to_file = QFileDialog::getOpenFileName(this, tr("Open file"));
    if(qstr_path_to_file.length()<3){
        return;
    }
    ui->lePathToLoadFile->setText(qstr_path_to_file);
    getFormSettings()->setButtonSaveEnabled(true);
}
void FormSettingsOnLoadFiles::onChangeData(){
    Params::_v_file_templates v_start_load_file_templates_new;
    v_start_load_file_templates_new.emplace_back(ui->lePathToLoadFile->text().toStdString(), ui->cbTemplate->currentText().toStdString() );
    Params::GetInstance()->setStartLoadFileTemplates(v_start_load_file_templates_new);
    getFormSettings()->setAppSettingsChanged();
}

