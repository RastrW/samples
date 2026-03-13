#include "checkboxListDialog.h"
#include <QCheckBox>
#include <QTableWidget>

CheckboxListDialog::CheckboxListDialog(int checkboxColumn, QWidget* parent)
    : QDialog(parent)
    , m_checkboxColumn(checkboxColumn)
{
    m_cbSelectAll = new QCheckBox(this);
    m_cbSelectAll->setTristate(true);

    m_twList = new QTableWidget(this);
}

void CheckboxListDialog::initCheckboxControls(const QString& selectAllLabel)
{
    m_cbSelectAll->setText(selectAllLabel);

    // clicked срабатывает ПОСЛЕ смены состояния Qt'ом.
    // Нас не устраивает автоматический переход PartiallyChecked → Unchecked,
    // поэтому читаем текущее состояние и корректируем его вручную.
    connect(m_cbSelectAll, &QCheckBox::clicked, this, [this]() {
        selectAllToggled(m_cbSelectAll->checkState());
    });

    connect(m_twList, &QTableWidget::itemChanged,
            this, &CheckboxListDialog::slot_itemChanged);
}

void CheckboxListDialog::selectAllToggled(Qt::CheckState currentState)
{
    // Unchecked  → пользователь снял → снимаем все
    // Checked / PartiallyChecked → выбираем все
    const Qt::CheckState newState =
        (currentState == Qt::Unchecked) ? Qt::Unchecked : Qt::Checked;

    m_bUpdating = true;
    m_cbSelectAll->setCheckState(newState);
    for (int row = 0; row < m_twList->rowCount(); ++row) {
        QTableWidgetItem* item = m_twList->item(row, m_checkboxColumn);
        if (item)
            item->setCheckState(newState);
    }
    m_bUpdating = false;
}

void CheckboxListDialog::slot_itemChanged(QTableWidgetItem* item)
{
    if (m_bUpdating)
        return;
    if (item->column() != m_checkboxColumn)
        return;

    int nChecked = 0;
    const int nTotal = m_twList->rowCount();
    for (int row = 0; row < nTotal; ++row) {
        const QTableWidgetItem* cb = m_twList->item(row, m_checkboxColumn);
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

void CheckboxListDialog::updateSelectAll()
{
    // Принудительно пересчитать состояние без флага m_bUpdating —
    // полезно после программного заполнения таблицы.
    QTableWidgetItem stub;          // фиктивный item в нужной колонке
    stub.setData(Qt::UserRole, {});
    // Просто вызовем логику через любую ячейку чекбоксной колонки
    if (m_twList->rowCount() > 0) {
        if (auto* cell = m_twList->item(0, m_checkboxColumn))
            slot_itemChanged(cell);
    }
}
