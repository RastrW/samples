#include "checkableTableWidget.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <vector>
#include <string>

CheckableTableWidget::CheckableTableWidget(QWidget* parent)
    : SettingsStackedItemWidget(parent)
{
    setupUI();
}

void CheckableTableWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({tr("Выбрать"), tr("Имя")});
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setVisible(false);

    layout->addWidget(m_table);
    setLayout(layout);

    connect(m_table, &QTableWidget::itemChanged,
            this,    &CheckableTableWidget::onItemChanged);
}

void CheckableTableWidget::populate(const std::vector<std::string>& allItems,
                                    const std::vector<std::string>& checkedItems)
{
    // Блокируем сигналы, чтобы заполнение не триггерило settingsChanged()
    QSignalBlocker blocker(m_table);

    m_table->setRowCount(0);

    int row = 0;
    for (const auto& item : allItems) {
        m_table->insertRow(row);

        bool isChecked = std::find(checkedItems.begin(), checkedItems.end(), item)
                         != checkedItems.end();

        auto* checkItem = new QTableWidgetItem();
        checkItem->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        m_table->setItem(row, COL_CHECK, checkItem);

        auto* nameItem = new QTableWidgetItem(QString::fromStdString(item));
        nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); // только чтение
        m_table->setItem(row, COL_NAME, nameItem);

        ++row;
    }

    m_table->resizeColumnsToContents();
}

std::vector<std::string> CheckableTableWidget::checkedItems() const {
    std::vector<std::string> result;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        auto* checkItem = m_table->item(row, COL_CHECK);
        if (checkItem && checkItem->checkState() == Qt::Checked) {
            if (auto* nameItem = m_table->item(row, COL_NAME)) {
                result.emplace_back(nameItem->text().toStdString());
            }
        }
    }
    return result;
}

void CheckableTableWidget::onItemChanged(QTableWidgetItem* item) {
    if (item->column() == COL_CHECK) {
        emit settingsChanged();
    }
}