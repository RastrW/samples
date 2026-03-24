#include "selectionDialog.h"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>

SelectionDialog::SelectionDialog(std::string selection, std::string colName, QWidget *parent)
    : QDialog(parent), m_selection(selection), m_colName(colName)
{
    setWindowTitle(tr("Выборка"));
    setWindowModality(Qt::ApplicationModal);
    setSizeGripEnabled(false);

    QLabel*   label    = new QLabel(tr("Выборка"), this);
    m_textEdit           = new QLineEdit(this);

    if (m_selection.empty()) {
        m_textEdit->setPlaceholderText(
            "Пример для выбранного столбца: " + QString::fromStdString(m_colName) + ">10");
    } else {
        m_textEdit->setPlaceholderText(m_selection.c_str());
    }
    m_textEdit->setFocus();
    m_textEdit->setMinimumWidth(300);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &SelectionDialog::on_buttonBox_accepted);
    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(label);
    mainLayout->addWidget(m_textEdit);
    mainLayout->addWidget(buttonBox);
}

void SelectionDialog::on_buttonBox_accepted(){

    emit sig_selectionAccepted(m_textEdit->text().toStdString());
}
