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

    m_cbSelectAll = new QCheckBox(tr("Отметить всё"));
    m_cbSelectAll->setTristate(true);

    m_twTemplates = new QTableWidget();
    m_twTemplates->setColumnCount(2);
    m_twTemplates->setSelectionMode(QAbstractItemView::SingleSelection);
    m_twTemplates->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_twTemplates->setHorizontalHeaderLabels(QStringList() << "" << "Template");
    m_twTemplates->horizontalHeader()->setStretchLastSection(true);
    m_twTemplates->verticalHeader()->setVisible(false);

    const Params::_v_template_exts v_template_ext{ Params::get_instance()->getTemplateExts() };
    int n_row_num = 0;
    for (const auto& template_ext : v_template_ext) {
        m_twTemplates->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->setCheckState(Qt::Unchecked);
        m_twTemplates->setItem(n_row_num, n_colnum_checked_, ptwi_checkbox);

        QTableWidgetItem* ptwi_name = new QTableWidgetItem(
            QString("%1%2").arg(template_ext.first.c_str()).arg(template_ext.second.c_str()));
        ptwi_name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_twTemplates->setItem(n_row_num, n_colnum_templatename_, ptwi_name);
        n_row_num++;
    }
    m_twTemplates->resizeColumnsToContents();
    m_twTemplates->resizeRowsToContents();

    connect(m_cbSelectAll, &QCheckBox::clicked, this, [this]() {
        // clicked срабатывает ПОСЛЕ смены состояния Qt'ом.
        // Нас не устраивает автоматический переход PartiallyChecked→Unchecked,
        // поэтому читаем текущее состояние и корректируем его вручную.
        const Qt::CheckState currentState = m_cbSelectAll->checkState();
        selectAllToggled(currentState);
    });
    connect(m_twTemplates, &QTableWidget::itemChanged,
            this, &FileNewDialog::slot_itemChanged);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_cbSelectAll);
    mainLayout->addWidget(m_twTemplates);
    mainLayout->addWidget(buttonBox);
}

void FileNewDialog::selectAllToggled(Qt::CheckState currentState){

    Qt::CheckState newState;
    if (currentState == Qt::Unchecked) {
        // Был Checked → пользователь снял → снимаем все
        newState = Qt::Unchecked;
    } else {
        // Был Unchecked или PartiallyChecked → выбираем все
        newState = Qt::Checked;
    }

    // Явно фиксируем состояние cbSelectAll (важно для случая PartiallyChecked)
    m_bUpdating = true;
    m_cbSelectAll->setCheckState(newState);
    for (int row = 0; row < m_twTemplates->rowCount(); ++row) {
        QTableWidgetItem* item = m_twTemplates->item(row, n_colnum_checked_);
        if (item)
            item->setCheckState(newState);
    }
    m_bUpdating = false;
}

void FileNewDialog::slot_itemChanged(QTableWidgetItem* item)
{
    if (m_bUpdating)
        return;
    if (item->column() != n_colnum_checked_)
        return;

    int nChecked = 0;
    const int nTotal = m_twTemplates->rowCount();
    for (int row = 0; row < nTotal; ++row) {
        const QTableWidgetItem* cb = m_twTemplates->item(row, n_colnum_checked_);
        if (cb && cb->checkState() == Qt::Checked)
            ++nChecked;
    }

    m_bUpdating = true;
    if (nChecked == 0)
        m_cbSelectAll->setCheckState(Qt::Unchecked);
    else if (nChecked == nTotal)
        m_cbSelectAll->setCheckState(Qt::Checked);
    else
        m_cbSelectAll->setCheckState(Qt::PartiallyChecked);
    m_bUpdating = false;
}

FileNewDialog::_s_checked_templatenames FileNewDialog::getCheckedTemplateNames() const
{
    _s_checked_templatenames s_checked_templatenames;
    for (int n_rownum = 0; n_rownum < m_twTemplates->rowCount(); n_rownum++) {
        const QTableWidgetItem* ptwi_checkbox = m_twTemplates->item(n_rownum, n_colnum_checked_);
        if (Qt::Checked == ptwi_checkbox->checkState()) {
            const QTableWidgetItem* ptwi_name = m_twTemplates->item(n_rownum, n_colnum_templatename_);
            s_checked_templatenames.emplace(ptwi_name->text().toStdString());
        }
    }
    return s_checked_templatenames;
}
