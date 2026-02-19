#include "formsettingsonloadforms.h"
#include "ui_formsettingsonloadforms.h"
#include "params.h"

FormSettingsOnLoadForms::FormSettingsOnLoadForms(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSettingsOnLoadForms){
    ui->setupUi(this);
    ui->twForms->setColumnCount(2);
    ui->twForms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twForms->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twForms->horizontalHeader()->setStretchLastSection(true);
    ui->twForms->resizeColumnsToContents();
    ui->twForms->resizeRowsToContents();
    ui->twForms->verticalHeader()->setVisible(false); // hide row numbers
    ui->twForms->horizontalHeader()->setVisible(false);
}
void FormSettingsOnLoadForms::showEvent( QShowEvent* event ){
    ui->twForms->setRowCount(0);
    int n_row_num = 0;
    for(const auto& form_exists_name : Params::get_instance()->getFormsExists()){
        ui->twForms->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->data(Qt::CheckStateRole);
        ptwi_checkbox->setCheckState(Qt::Unchecked);
        for(const auto& form_load_name: Params::get_instance()->getStartLoadForms()){
            if(form_exists_name == form_load_name){
                ptwi_checkbox->setCheckState(Qt::Checked);
            }
        }
        ui->twForms->setItem( n_row_num, n_colnum_checked_, ptwi_checkbox );
        QTableWidgetItem* ptwi_formname = new QTableWidgetItem( QString("%1").arg(form_exists_name.c_str()) );
        ptwi_formname->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->twForms->setItem( n_row_num, n_colnum_formname_, ptwi_formname );
        n_row_num++;
    }
}
FormSettingsOnLoadForms::~FormSettingsOnLoadForms(){
    delete ui;
}
void FormSettingsOnLoadForms::on_pbApply_clicked(){
    Params::_v_forms v_forms_load_new;
    for( int n_rownum = 0; n_rownum < ui->twForms->rowCount() ; n_rownum++ ){
        const QTableWidgetItem* ptwi_checkbox = ui->twForms->item( n_rownum, n_colnum_checked_ );
        if(Qt::Checked == ptwi_checkbox->checkState()){
            const QTableWidgetItem* ptwi_formname = ui->twForms->item( n_rownum, n_colnum_formname_ );
            v_forms_load_new.emplace_back(ptwi_formname->text().toStdString());
        }
    }
    Params::get_instance()->setStartLoadForms(v_forms_load_new);
}

