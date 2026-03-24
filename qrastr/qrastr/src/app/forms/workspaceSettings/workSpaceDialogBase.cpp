#include "workspacedialogbase.h"

WorkspaceDialogBase::WorkspaceDialogBase(const QStringList &workspaces,
                                         QWidget *parent)
    : QDialog(parent)
    , m_listWidget(new QListWidget(this))
    , m_buttonBox(new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
    , m_mainLayout(new QVBoxLayout(this))
{
    m_listWidget->addItems(workspaces);

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
    for (QWidget *w : m_aboveWidgets)
        m_mainLayout->addWidget(w);

    m_mainLayout->addWidget(m_listWidget);

    for (QWidget *w : m_belowWidgets)
        m_mainLayout->addWidget(w);

    m_mainLayout->addWidget(m_buttonBox);
}
