#include "loadWorkspaceDialog.h"

#include <QMessageBox>
#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QHeaderView>

LoadWorkspaceDialog::LoadWorkspaceDialog(const QStringList &workspaces,
                                         const QString     &startupName,
                                         QWidget           *parent)
    : WorkSpaceDialogBase(workspaces, parent)
{
    setWindowTitle(tr("Загрузить рабочую область"));

    setupColumns(startupName);  // расширяем дерево до двух колонок
    finalizeLayout();

    disconnect(m_buttonBox, &QDialogButtonBox::accepted,
               this,        &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this,        &LoadWorkspaceDialog::onOkClicked);

    // Двойной клик — быстрое подтверждение
    connect(m_tree, &QTreeWidget::itemDoubleClicked,
            this,   [this](QTreeWidgetItem *) { onOkClicked(); });

    resize(420, 300);
}

void LoadWorkspaceDialog::setupColumns(const QString &startupName)
{
    // Добавляем вторую колонку
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels({tr("Рабочая область"), tr("При старте")});
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_tree->header()->resizeSection(1, 90);

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_tree->topLevelItem(i);

        if (item->text(0) == startupName) {
            item->setText(1, tr("✓"));
            item->setTextAlignment(1, Qt::AlignCenter);
        }

        // Запрещаем редактирование обеих колонок
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

QString LoadWorkspaceDialog::selectedWorkspace() const
{
    const QTreeWidgetItem *item = m_tree->currentItem();
    return item ? item->text(0) : QString{};
}

void LoadWorkspaceDialog::onOkClicked()
{
    const QString name = selectedWorkspace();

    if (name.isEmpty()) {
        QMessageBox::information(this,
                                 tr("Выбор не выполнен"),
                                 tr("Пожалуйста, выберите рабочую область из списка."));
        return;
    }

    emit workspaceSelected(name);
    accept();
}
