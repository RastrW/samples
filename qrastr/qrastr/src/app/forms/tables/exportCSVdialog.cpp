#include "exportCSVdialog.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLabel>
#include "table/ITableRepository.h"

ExportCSVdialog::ExportCSVdialog(ITableRepository* repo,
                                 const std::string& tableName,
                                 const std::string& defaultCols,
                                 QWidget* parent)
    : QDialog(parent), m_repo(repo)
{
    setWindowTitle(tr("Экспорт в формате CSV"));

    // --- GroupBox "Файл:" ---
    QGroupBox*   groupBox  = new QGroupBox(tr("Файл:"), this);
    m_lePath               = new QLineEdit();
    QPushButton* pushButton = new QPushButton("...");
    pushButton->setFixedWidth(32);
    connect(pushButton, &QPushButton::clicked, this, &ExportCSVdialog::on_pushButton_clicked);

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
    m_leTable = new QLineEdit(QString::fromStdString(tableName));
    m_leParam = new QLineEdit(QString::fromStdString(defaultCols));
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
            this, &ExportCSVdialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(formLay);
    mainLayout->addWidget(buttonBox);
}

void ExportCSVdialog::on_pushButton_clicked()
{
    QFileDialog fileDlg(this);
    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setWindowTitle("Сохранение");
    fileDlg.setDirectory(QDir::currentPath());
    if (fileDlg.exec() == QDialog::Rejected) return;
    m_path = (fileDlg.selectedFiles())[0].toStdString();
    m_lePath->setText(m_path.c_str());
}

void ExportCSVdialog::accept()
{
    eCSVCode kod = eCSVCode::Replace;
    if (m_rbExtend->isChecked())          kod = eCSVCode::KeyAdd;
    if (m_rbOverwriteHeaders->isChecked()) kod = eCSVCode::ReplaceName;

    m_repo->exportToCsv(
        m_leTable->text().toStdString(),
        m_leParam->text().toStdString(),
        m_leSelection->text().toStdString(),
        m_path,
        m_leSeparator->text().toStdString(),
        kod);
    close();
}
