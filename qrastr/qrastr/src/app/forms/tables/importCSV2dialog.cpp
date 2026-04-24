#include "importCSV2dialog.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include "astra/IPlainRastr.h"
#include "table/rtablesdatamanager.h"

ImportCSV2dialog::ImportCSV2dialog(ITableRepository*  repo,
                                   const std::string& tableName,
                                   const std::string& defaultCols,
                                   QWidget*           parent)
    : QDialog(parent), m_repo(repo)
{
    setWindowTitle(tr("Импорт из формата CSV"));

    // --- GroupBox "Файл" ---
    QGroupBox*   groupBox   = new QGroupBox(tr("Файл"), this);
    m_leFile           = new QLineEdit();
    QPushButton* pushButton = new QPushButton("...");
    pushButton->setFixedWidth(32);
    connect(pushButton, &QPushButton::clicked, this, &ImportCSV2dialog::on_pushButton_clicked);

    m_rbAdd    = new QRadioButton(tr("Присоединить"));
    m_rbLoad   = new QRadioButton(tr("Загрузить"));
    m_rbUnion  = new QRadioButton(tr("Объединить"));
    m_rbUpdate = new QRadioButton(tr("Обновить"));
    m_rbLoad2  = new QRadioButton(tr("Загр (2)"));
    m_rbUpdate->setChecked(true);

    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(m_leFile);
    fileRow->addWidget(pushButton);

    QGridLayout* radioGrid = new QGridLayout();
    radioGrid->addWidget(m_rbAdd,    0, 0);
    radioGrid->addWidget(m_rbUnion,  0, 1);
    radioGrid->addWidget(m_rbLoad2,  0, 2);
    radioGrid->addWidget(m_rbLoad,   1, 0);
    radioGrid->addWidget(m_rbUpdate, 1, 1);

    QVBoxLayout* gbLay = new QVBoxLayout(groupBox);
    gbLay->addLayout(fileRow);
    gbLay->addLayout(radioGrid);

    // --- Fields below groupBox ---
    m_leTname  = new QLineEdit(QString::fromStdString(tableName));
    m_leDivider   = new QLineEdit(";");
    m_leDivider->setFixedWidth(50);
    m_leParams = new QLineEdit(QString::fromStdString(defaultCols));
    m_leSelection   = new QLineEdit();
    m_leByDefault = new QLineEdit();

    QFormLayout* formLay = new QFormLayout();
    QHBoxLayout* tableRow = new QHBoxLayout();
    tableRow->addWidget(m_leTname);
    tableRow->addWidget(new QLabel(tr("Разделитель")));
    tableRow->addWidget(m_leDivider);
    formLay->addRow(tr("Таблица:"),         tableRow);
    formLay->addRow(tr("Параметры:"),       m_leParams);
    formLay->addRow(tr("Выборка:"),         m_leSelection);
    formLay->addRow(tr("По умолчанию:"),    m_leByDefault);

    // --- ButtonBox ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &ImportCSV2dialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(formLay);
    mainLayout->addWidget(buttonBox);
}

void ImportCSV2dialog::on_pushButton_clicked()
{
    QFileDialog fileDlg(this);
    fileDlg.setAcceptMode(QFileDialog::AcceptOpen);
    fileDlg.setWindowTitle("Открытие");
    fileDlg.setDirectory(QDir::currentPath());
    if (fileDlg.exec() == QDialog::Rejected) {return;}
    m_leFile->setText((fileDlg.selectedFiles())[0]);
}

void ImportCSV2dialog::on_buttonBox_accepted()
{
    eCSVCode kod = eCSVCode::Key;  // Обновить — по умолчанию
    if (m_rbAdd->isChecked())    kod = eCSVCode::Add;
    if (m_rbLoad->isChecked())   kod = eCSVCode::Replace;
    if (m_rbLoad2->isChecked())  kod = eCSVCode::Index;
    if (m_rbUnion->isChecked())  kod = eCSVCode::KeyAdd;
    if (m_rbUpdate->isChecked()) kod = eCSVCode::Key;

    m_repo->importToCsv(
        m_leTname->text().toStdString(),
        m_leParams->text().toStdString(),
        m_leSelection->text().toStdString(),
        m_leFile->text().toStdString(),
        m_leDivider->text().toStdString(),
        m_leByDefault->text().toStdString(),
        kod);

    close();
}

