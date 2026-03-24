#include "loadworkspacedialog.h"

#include <QMessageBox>

LoadWorkspaceDialog::LoadWorkspaceDialog(const QStringList &workspaces,
                                         QWidget           *parent)
    : WorkspaceDialogBase(workspaces, parent)
{
    setWindowTitle(tr("Загрузить рабочую область"));

    // Дополнительных виджетов нет — сразу завершаем компоновку
    finalizeLayout();

    // Переопределяем обработчик Ok для проверки выбора и отправки сигнала
    disconnect(m_buttonBox, &QDialogButtonBox::accepted,
               this,        &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this,        &LoadWorkspaceDialog::onOkClicked);

    // Двойной клик — быстрое подтверждение
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this,         [this](QListWidgetItem *) { onOkClicked(); });

    resize(400, 300);
}

// ─── Публичный интерфейс ─────────────────────────────────────────────────────

QString LoadWorkspaceDialog::selectedWorkspace() const
{
    const QListWidgetItem *item = m_listWidget->currentItem();
    return item ? item->text() : QString{};
}

// ─── Приватные слоты ─────────────────────────────────────────────────────────

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
