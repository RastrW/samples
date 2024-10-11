#include "formsettingsonloadtemplates.h"
#include "ui_formsettingsonloadtemplates.h"
#include "params.h"

FormSettingsOnLoadTemplates::FormSettingsOnLoadTemplates(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormSettingsOnLoadTemplates){
    ui->setupUi(this);
    ui->twTemplates->setColumnCount(2);
    //ui->twTemplates->setShowGrid(true);
    ui->twTemplates->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twTemplates->setSelectionBehavior(QAbstractItemView::SelectRows);
    //QStringList headers;
    //headers<<""<<"Template";
    //ui->twTemplates->setHorizontalHeaderLabels(headers);
    ui->twTemplates->horizontalHeader()->setStretchLastSection(true);
    const Params::_v_templates v_templates{ Params::GetInstance()->getTemplates() };
    const Params::_v_template_exts v_template_ext{ Params::GetInstance()->getTemplateExts() };
    int n_row_num = 0;
    for(const Params::_v_template_exts::value_type& template_ext : v_template_ext){
        ui->twTemplates->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->data(Qt::CheckStateRole);
        ptwi_checkbox->setCheckState(Qt::Unchecked);
        for(const Params::_v_templates::value_type& templ : v_templates){
            const std::string str_templ_in_table{ template_ext.first + template_ext.second };
            if(str_templ_in_table == templ){
                ptwi_checkbox->setCheckState(Qt::Checked);
            }
        }
        ui->twTemplates->setItem( n_row_num, n_colnum_checked_, ptwi_checkbox );
        QTableWidgetItem* ptwi_templatename = new QTableWidgetItem( QString("%1%2").arg(template_ext.first.c_str()).arg(template_ext.second.c_str()) );
        ptwi_templatename->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->twTemplates->setItem( n_row_num, n_colnum_templatename_, ptwi_templatename );
        n_row_num++;
    }
    ui->twTemplates->resizeColumnsToContents();
    ui->twTemplates->resizeRowsToContents();
    ui->twTemplates->verticalHeader()->setVisible(false); // hide row numbers
    ui->twTemplates->horizontalHeader()->setVisible(false);
}
FormSettingsOnLoadTemplates::~FormSettingsOnLoadTemplates(){
    delete ui;
}
void FormSettingsOnLoadTemplates::on_pbApplay_clicked(){
    Params::_v_templates v_templates_new;
    for( int n_rownum = 0; n_rownum < ui->twTemplates->rowCount() ; n_rownum++ ){
        const QTableWidgetItem* ptwi_checkbox = ui->twTemplates->item( n_rownum, n_colnum_checked_ );
        if(Qt::Checked == ptwi_checkbox->checkState()){
            const QTableWidgetItem* ptwi_templatename = ui->twTemplates->item( n_rownum, n_colnum_templatename_ );
            v_templates_new.emplace_back(ptwi_templatename->text().toStdString());
        }
    }
    Params::GetInstance()->setTemplates(v_templates_new);
}

