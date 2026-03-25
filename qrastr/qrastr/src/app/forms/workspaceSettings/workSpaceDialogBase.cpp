#include "workspacedialogbase.h"
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QStringList>

WorkspaceDialogBase::WorkspaceDialogBase(const QStringList &workspaces,
                                         QWidget           *parent)
    : QDialog(parent)
    , m_tree(new QTreeWidget(this))
    , m_buttonBox(new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
    , m_mainLayout(new QVBoxLayout(this))
{
    // Базовая настройка дерева — одна колонка с именем.
    // Производные классы могут добавить колонки до вызова finalizeLayout().
    m_tree->setColumnCount(1);
    m_tree->setHeaderLabel(tr("Рабочая область"));
    m_tree->setRootIsDecorated(false);

    for (const QString &name : workspaces) {
        auto *item = new QTreeWidgetItem(m_tree);
        item->setText(0, name);
    }

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void WorkspaceDialogBase::insertWidgetAboveList(QWidget *widget)
{
    m_aboveWidgets.append(widget);
}

void WorkspaceDialogBase::insertWidgetBelowList(QWidget *widget)
{
    m_belowWidgets.append(widget);
}

void WorkspaceDialogBase::finalizeLayout()
{
    for (auto* w : m_aboveWidgets){
        m_mainLayout->addWidget(w);
    }

    m_mainLayout->addWidget(m_tree);

    for (auto* w : m_belowWidgets){
        m_mainLayout->addWidget(w);
    }

    m_mainLayout->addWidget(m_buttonBox);
}
