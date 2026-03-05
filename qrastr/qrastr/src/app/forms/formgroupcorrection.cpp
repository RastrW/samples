#include "formgroupcorrection.h"
#include <QRadioButton>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "rdata.h"
#include "rcol.h"

formgroupcorrection::formgroupcorrection(RData* _prdata, RCol* _prcol, QWidget *parent)
    : QDialog(parent), m_prdata{_prdata}, m_prcol {_prcol}
{
    setWindowTitle(tr("Групповая коррекция"));

    // --- Row 1: Параметр + comboBox + label_3 ---
    QComboBox* cbParameters = new QComboBox();
    QLabel* lTable = new QLabel();

    QString str_lab3 = "в таблице ";
    str_lab3.append(_prdata->t_name_.c_str());
    lTable->setText(str_lab3);

    std::string str_par = _prdata->t_name_;
    str_par.append("[").append(_prdata->t_title_).append("]."); //node[Узлы].
    for (auto& col : *m_prdata) {
        std::string par = str_par;
        par.append(col.getColName()).append("[").append(col.getTitle()).append("]");
        cbParameters->addItem(par.c_str());
    }
    cbParameters->setCurrentIndex(m_prcol->getIndex());

    QHBoxLayout* row1 = new QHBoxLayout();
    row1->addWidget(new QLabel(tr("Параметр")));
    row1->addWidget(cbParameters);
    row1->addWidget(lTable);

    // --- Row 2: "radioButton Удалять строки" ---
    QRadioButton* rbDelete = new QRadioButton(tr("Удалять строки"));

    // --- Row 3: radioButton + expression ---
    QRadioButton* rbFormula = new QRadioButton(tr("Формула расчета:"));
    rbDelete->setChecked(true);
    m_leExpression = new QLineEdit();

    QHBoxLayout* row3 = new QHBoxLayout();
    row3->addWidget(rbFormula);
    row3->addWidget(m_leExpression);

    // --- Row 4: Выборка ---
    m_leSelection = new QLineEdit();

    QHBoxLayout* row4 = new QHBoxLayout();
    row4->addWidget(new QLabel(tr("Выборка")));
    row4->addWidget(m_leSelection);

    // --- ButtonBox ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &formgroupcorrection::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(row1);
    mainLayout->addWidget(rbDelete);
    mainLayout->addLayout(row3);
    mainLayout->addLayout(row4);
    mainLayout->addWidget(buttonBox);
}

void formgroupcorrection::on_buttonBox_accepted()
{
    m_selection  = m_leExpression->text().toStdString();
    m_expression = m_leExpression->text().toStdString();
    m_prcol->calc(m_prdata->getAstra(), m_expression, m_selection);
    close();
}
