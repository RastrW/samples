#include "fileNewDialog.h"
#include "params.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QVBoxLayout>

FileNewDialog::FileNewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Открыть новый файл"));
    //setWindowIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
    setWindowIcon(QIcon::fromTheme("document-new"));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(316, 442);

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

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(twTemplates);
    mainLayout->addWidget(buttonBox);
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
