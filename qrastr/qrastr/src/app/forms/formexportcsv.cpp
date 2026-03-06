#include "formexportcsv.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLabel>
#include "qastra.h"
#include "rdata.h"
#include <astra/IPlainRastrWrappers.h>

formexportcsv::formexportcsv(RData* _prdata, QWidget *parent)
    : QDialog(parent), prdata_(_prdata)
{
    setWindowTitle(tr("Экспорт в формате CSV"));

    // --- GroupBox "Файл:" ---
    QGroupBox*   groupBox  = new QGroupBox(tr("Файл:"), this);
    m_lePath               = new QLineEdit();
    QPushButton* pushButton = new QPushButton("...");
    pushButton->setFixedWidth(32);
    connect(pushButton, &QPushButton::clicked, this, &formexportcsv::on_pushButton_clicked);

    m_rbOverwrite   = new QRadioButton(tr("Перезаписать"));
    m_rbExtend = new QRadioButton(tr("Дополнить"));
    m_rbOverwriteHeaders = new QRadioButton(tr("Перезаписать (заголовки)"));
    m_rbOverwrite->setChecked(true);

    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(m_lePath);
    fileRow->addWidget(pushButton);

    QHBoxLayout* radioRow = new QHBoxLayout();
    radioRow->addWidget(m_rbOverwrite);
    radioRow->addWidget(m_rbExtend);
    radioRow->addWidget(m_rbOverwriteHeaders);

    QVBoxLayout* gbLay = new QVBoxLayout(groupBox);
    gbLay->addLayout(fileRow);
    gbLay->addLayout(radioRow);

    // --- Нижняя часть ---
    m_leTable = new QLineEdit(_prdata->t_name_.c_str());
    m_leParam = new QLineEdit(_prdata->get_cols().c_str());
    m_leSeparator = new QLineEdit(";");
    m_leSeparator->setFixedWidth(50);
    m_leSelection = new QLineEdit();

    QFormLayout* formLay = new QFormLayout();
    QHBoxLayout* tableRow = new QHBoxLayout();
    tableRow->addWidget(m_leTable);
    tableRow->addWidget(new QLabel(tr("Разделитель:")));
    tableRow->addWidget(m_leSeparator);
    formLay->addRow(tr("Таблица:"),   tableRow);
    formLay->addRow(tr("Параметры:"), m_leParam);
    formLay->addRow(tr("Выборка"),    m_leSelection);

    // --- ButtonBox ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &formexportcsv::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(formLay);
    mainLayout->addWidget(buttonBox);
}

void formexportcsv::on_pushButton_clicked()
{
    QFileDialog fileDlg(this);
    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setWindowTitle("Сохранение");
    fileDlg.setDirectory(QString::fromStdString(std::filesystem::current_path().string()));
    if (fileDlg.exec() == QDialog::Rejected) return;
    m_path = (fileDlg.selectedFiles())[0].toStdString();
    m_lePath->setText(m_path.c_str());
}

void formexportcsv::accept()
{
    std::string table_name = m_leTable->text().toStdString();
    std::string div        = m_leSeparator->text().toStdString();
    std::string cols       = m_leParam->text().toStdString();
    std::string selection  = m_leSelection->text().toStdString();

    IRastrTablesPtr tablesx{ prdata_->getAstra()->getRastr()->Tables() };
    IRastrTablePtr  table{ tablesx->Item(table_name) };
    if (!selection.empty())
        IRastrResultVerify{table->SetSelection(selection)};

    eCSVCode kod = eCSVCode::Replace;
    if (m_rbOverwrite->isChecked())   kod = eCSVCode::Replace;
    if (m_rbExtend->isChecked()) kod = eCSVCode::KeyAdd;
    if (m_rbOverwriteHeaders->isChecked()) kod = eCSVCode::ReplaceName;

    IRastrResultVerify{table->ToCsv(kod, m_path, cols, div)};
    close();
}
