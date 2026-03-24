#include "fileNewDialog.h"
#include "params.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>

FileNewDialog::FileNewDialog(QWidget *parent)
    : CheckboxListDialog(n_colnum_checked_, parent)
{
    setWindowTitle(tr("Открыть новый файл"));
    setWindowIcon(QIcon::fromTheme("document-new"));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(350, 450);

    m_twList->setColumnCount(2);
    m_twList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_twList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_twList->setHorizontalHeaderLabels(QStringList() << "" << "Template");
    m_twList->horizontalHeader()->setStretchLastSection(true);
    m_twList->verticalHeader()->setVisible(false);

    const Params::_v_template_exts v_template_ext{ Params::get_instance()->getTemplateExts() };
    int n_row_num = 0;
    for (const auto& template_ext : v_template_ext) {
        m_twList->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Unchecked);
        m_twList->setItem(n_row_num, n_colnum_checked_, ptwi_checkbox);

        QTableWidgetItem* ptwi_name = new QTableWidgetItem(
            QString("%1%2").arg(template_ext.first.c_str()).arg(template_ext.second.c_str()));
        ptwi_name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_twList->setItem(n_row_num, n_colnum_templatename_, ptwi_name);
        n_row_num++;
    }
    m_twList->resizeColumnsToContents();
    m_twList->resizeRowsToContents();

    initCheckboxControls(tr("Отметить всё"));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_cbSelectAll);
    mainLayout->addWidget(m_twList);
    mainLayout->addWidget(buttonBox);
}

FileNewDialog::_s_checked_templatenames FileNewDialog::getCheckedTemplateNames() const
{
    _s_checked_templatenames s_checked_templatenames;
    for (int n_rownum = 0; n_rownum < m_twList->rowCount(); n_rownum++) {
        const QTableWidgetItem* ptwi_checkbox = m_twList->item(n_rownum, n_colnum_checked_);
        if (Qt::Checked == ptwi_checkbox->checkState()) {
            const QTableWidgetItem* ptwi_name = m_twList->item(n_rownum, n_colnum_templatename_);
            s_checked_templatenames.emplace(ptwi_name->text().toStdString());
        }
    }
    return s_checked_templatenames;
}
