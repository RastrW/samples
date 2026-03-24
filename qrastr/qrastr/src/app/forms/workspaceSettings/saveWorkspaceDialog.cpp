#include "saveworkspacedialog.h"

#include <QLabel>
#include <QMessageBox>

SaveWorkspaceDialog::SaveWorkspaceDialog(const QStringList &workspaces,
                                         bool               loadOnStartup,
                                         QWidget           *parent)
    : WorkspaceDialogBase(workspaces, parent)
    , m_loadOnStartupCheck(new QCheckBox(tr("Загрузка при старте"), this))
{
    setWindowTitle(tr("Сохранить рабочую область"));

    // --- Флаг «Загрузка при старте» — над списком ---
    m_loadOnStartupCheck->setChecked(loadOnStartup);
    insertWidgetAboveList(m_loadOnStartupCheck);

    // --- Строка добавления — под списком ---
    setupAddWidget();
    insertWidgetBelowList(m_addRow);

    // Завершаем компоновку (список + buttonBox встают на свои места)
    finalizeLayout();

    // Переопределяем обработчик Ok, чтобы проверить ввод перед закрытием
    // Отключаем стандартную связку accept и подключаем свой слот
    disconnect(m_buttonBox, &QDialogButtonBox::accepted,
               this,        &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted,
            this,        &SaveWorkspaceDialog::onOkClicked);

    resize(400, 350);
}

// ─── Публичный интерфейс ─────────────────────────────────────────────────────

QString SaveWorkspaceDialog::newWorkspaceName() const
{
    return m_newWorkspaceName;
}

bool SaveWorkspaceDialog::loadOnStartup() const
{
    return m_loadOnStartupCheck->isChecked();
}

// ─── Приватные слоты ─────────────────────────────────────────────────────────

void SaveWorkspaceDialog::onAddClicked()
{
    // Переключаем видимость строки ввода
    const bool visible = !m_nameEdit->isVisible();
    m_nameEdit->setVisible(visible);

    if (visible) {
        m_nameEdit->setFocus();
        m_addButton->setText(tr("Скрыть"));
    } else {
        m_addButton->setText(tr("Добавить"));
    }
}

void SaveWorkspaceDialog::onOkClicked()
{
    const QString name = m_nameEdit->text().trimmed();

    if (!name.isEmpty()) {
        // Проверяем дублирование
        for (int i = 0; i < m_listWidget->count(); ++i) {
            if (m_listWidget->item(i)->text().compare(
                    name, Qt::CaseInsensitive) == 0)
            {
                QMessageBox::warning(this,
                                     tr("Дублирование"),
                                     tr("Рабочая область «%1» уже существует.")
                                         .arg(name));
                m_nameEdit->setFocus();
                m_nameEdit->selectAll();
                return;
            }
        }
        m_newWorkspaceName = name;
    }

    accept();
}

// ─── Вспомогательные методы ──────────────────────────────────────────────────

void SaveWorkspaceDialog::setupAddWidget()
{
    m_addRow   = new QWidget(this);
    m_nameEdit = new QLineEdit(m_addRow);
    m_nameEdit->setPlaceholderText(tr("Введите название рабочей области…"));
    m_nameEdit->setVisible(false);   // скрыто по умолчанию

    m_addButton = new QPushButton(tr("Добавить"), m_addRow);

    auto *rowLayout = new QHBoxLayout(m_addRow);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->addWidget(m_nameEdit);
    rowLayout->addWidget(m_addButton);

    connect(m_addButton, &QPushButton::clicked,
            this,        &SaveWorkspaceDialog::onAddClicked);

    // Enter в поле ввода — подтверждаем сразу как Ok
    connect(m_nameEdit, &QLineEdit::returnPressed,
            this,       &SaveWorkspaceDialog::onOkClicked);
}
