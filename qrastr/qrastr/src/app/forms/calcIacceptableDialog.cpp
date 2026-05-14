#include "calcIacceptableDialog.h"
#include <QGroupBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include "calculation/ICalculationEngine.h"

CalcIacceptableDialog::CalcIacceptableDialog(std::shared_ptr<ICalculationEngine> calcEngine,
                                             QWidget *parent)
    : QDialog(parent), m_calcEngine(calcEngine)
{
    setWindowTitle(tr("Расчет I доп от Т"));
    //Qt Layout-ы (такие как QHBoxLayout, QVBoxLayout) автоматически делают виджеты,
    //которые в них добавляются, дочерними элементами того виджета, к которому применен этот layout.
    // --- GroupBox "Параметры" ---
    QGroupBox* groupBox = new QGroupBox(tr("Параметры"), this);

    m_leTemperature   = new QLineEdit("25.0");
    m_leEmergencyLoading = new QLineEdit("0");
    m_leSelection = new QLineEdit();

    QHBoxLayout* hlay = new QHBoxLayout();
    hlay->addWidget(new QLabel(tr("Температура:")));
    hlay->addWidget(m_leTemperature);
    hlay->addWidget(new QLabel(tr("Аварийная загрузка (%):")));
    hlay->addWidget(m_leEmergencyLoading);

    QHBoxLayout* hlay2 = new QHBoxLayout();
    hlay2->addWidget(new QLabel(tr("Выборка:")));
    hlay2->addWidget(m_leSelection);

    QVBoxLayout* gbLay = new QVBoxLayout(groupBox);
    gbLay->addLayout(hlay);
    gbLay->addLayout(hlay2);

    // --- GroupBox "АС Метео" ---
    QGroupBox* groupBox_2 = new QGroupBox(tr("АС Метео"), this);

    QCheckBox* cdMeteo      = new QCheckBox(tr("Загрузить"));
    m_lData       = new QLabel(tr("Дата:"));
    m_dateTimeEdit  = new QDateTimeEdit();
    m_lData->setEnabled(false);
    m_dateTimeEdit->setEnabled(false);
    m_dateTimeEdit->setMinimumWidth(150);

    QHBoxLayout* gbLay2 = new QHBoxLayout(groupBox_2);
    gbLay2->addWidget(cdMeteo);
    gbLay2->addStretch();
    gbLay2->addWidget(m_lData);
    gbLay2->addWidget(m_dateTimeEdit);

    //connect(cdMeteo, &QCheckBox::checkStateChanged, this,
     //       &CalcIacceptableDialog::on_checkBox_checkStateChanged);
    connect(cdMeteo, &QCheckBox::stateChanged, this,
            &CalcIacceptableDialog::on_checkBox_checkStateChanged);

    // --- ButtonBox ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &CalcIacceptableDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // --- Main layout ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->addWidget(groupBox_2);
    mainLayout->addWidget(buttonBox);
}

void CalcIacceptableDialog::on_buttonBox_accepted()
{
    double temp   = m_leTemperature->text().toDouble();
    double emL  = m_leEmergencyLoading->text().toDouble();
    std::string sel = m_leSelection->text().toStdString();
    m_calcEngine->CalcIdop(temp, emL, sel);
    accept();
}

void CalcIacceptableDialog::on_checkBox_checkStateChanged(int state)
{
    bool enabled = (state == Qt::Checked);

    m_lData->setEnabled(enabled);
    m_dateTimeEdit->setEnabled(enabled);
}

void CalcIacceptableDialog::done(int result) {
    emit sig_calculationFinished();
    QDialog::done(result);
}


