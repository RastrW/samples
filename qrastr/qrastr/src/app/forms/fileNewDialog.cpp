#include "fileNewDialog.h"
#include "params.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>

FileNewDialog::FileNewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Открыть новый файл"));
    setWindowIcon(QIcon::fromTheme("document-new"));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(316, 442);

    cbSelectAll = new QCheckBox(tr("Отметить всё"));
    cbSelectAll->setTristate(true);

    twTemplates = new QTableWidget();
    twTemplates->setColumnCount(2);
    twTemplates->setSelectionMode(QAbstractItemView::SingleSelection);
    twTemplates->setSelectionBehavior(QAbstractItemView::SelectRows);
    twTemplates->setHorizontalHeaderLabels(QStringList() << "" << "Template");
    twTemplates->horizontalHeader()->setStretchLastSection(true);
    twTemplates->verticalHeader()->setVisible(false);

    const Params::_v_template_exts v_template_ext{ Params::get_instance()->getTemplateExts() };
    int n_row_num = 0;
    for (const auto& template_ext : v_template_ext) {
        twTemplates->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Unchecked);
        twTemplates->setItem(n_row_num, n_colnum_checked_, ptwi_checkbox);

        QTableWidgetItem* ptwi_name = new QTableWidgetItem(
            QString("%1%2").arg(template_ext.first.c_str()).arg(template_ext.second.c_str()));
        ptwi_name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        twTemplates->setItem(n_row_num, n_colnum_templatename_, ptwi_name);
        n_row_num++;
    }
    twTemplates->resizeColumnsToContents();
    twTemplates->resizeRowsToContents();

    connect(cbSelectAll, &QCheckBox::clicked, this, [this](bool checked) {
        onSelectAllToggled(checked ? Qt::Checked : Qt::Unchecked);
    });
    connect(twTemplates, &QTableWidget::itemChanged,
            this, &FileNewDialog::onItemChanged);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(cbSelectAll);
    mainLayout->addWidget(twTemplates);
    mainLayout->addWidget(buttonBox);
}

void FileNewDialog::onSelectAllToggled(Qt::CheckState state)
{
    // Промежуточное состояние пользователь не выбирает вручную —
    // при клике Qt переходит: Unchecked→Checked или Checked→Unchecked.
    // Если был PartiallyChecked (выставлен программно), следующий клик
    // переводит в Checked (выбрать все).
    if (state == Qt::PartiallyChecked)
        return;

    m_bUpdating = true;
    const Qt::CheckState newState = (state == Qt::Checked) ? Qt::Checked : Qt::Unchecked;
    for (int row = 0; row < twTemplates->rowCount(); ++row) {
        QTableWidgetItem* item = twTemplates->item(row, n_colnum_checked_);
        if (item)
            item->setCheckState(newState);
    }
    m_bUpdating = false;
}

void FileNewDialog::onItemChanged(QTableWidgetItem* item)
{
    if (m_bUpdating)
        return;
    if (item->column() != n_colnum_checked_)
        return;

    int nChecked = 0;
    int nTotal   = twTemplates->rowCount();
    for (int row = 0; row < nTotal; ++row) {
        const QTableWidgetItem* cb = twTemplates->item(row, n_colnum_checked_);
        if (cb && cb->checkState() == Qt::Checked)
            ++nChecked;
    }

    m_bUpdating = true;
    if (nChecked == 0)
        cbSelectAll->setCheckState(Qt::Unchecked);
    else if (nChecked == nTotal)
        cbSelectAll->setCheckState(Qt::Checked);
    else
        cbSelectAll->setCheckState(Qt::PartiallyChecked);
    m_bUpdating = false;
}

FileNewDialog::_s_checked_templatenames FileNewDialog::getCheckedTemplateNames() const
{
    _s_checked_templatenames s_checked_templatenames;
    for (int n_rownum = 0; n_rownum < twTemplates->rowCount(); n_rownum++) {
        const QTableWidgetItem* ptwi_checkbox = twTemplates->item(n_rownum, n_colnum_checked_);
        if (Qt::Checked == ptwi_checkbox->checkState()) {
            const QTableWidgetItem* ptwi_name = twTemplates->item(n_rownum, n_colnum_templatename_);
            s_checked_templatenames.emplace(ptwi_name->text().toStdString());
        }
    }
    return s_checked_templatenames;
}
