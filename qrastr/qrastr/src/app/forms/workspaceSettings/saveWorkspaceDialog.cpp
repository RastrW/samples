#include "saveworkspacedialog.h"

#include <QLabel>
#include <QMessageBox>

SaveWorkspaceDialog::SaveWorkspaceDialog(const QStringList &workspaces,
                                         QWidget           *parent)
    : WorkspaceDialogBase(workspaces, parent)
    , m_loadOnStartupCheck(new QCheckBox(tr("Загрузка при старте"), this))
    , m_deleteButton(new QPushButton(tr("Удалить"), this))
{
    setWindowTitle(tr("Сохранить рабочую область"));

    m_loadOnStartupCheck->setChecked(false);
    insertWidgetAboveList(m_loadOnStartupCheck);

    setupAddWidget();
    insertWidgetBelowList(m_addRow);
    insertWidgetBelowList(m_deleteButton);

    finalizeLayout();

    disconnect(m_buttonBox, &QDialogButtonBox::accepted,
               this,        &QDialog::accept);
    connect(m_buttonBox,  &QDialogButtonBox::accepted,
            this,         &SaveWorkspaceDialog::onOkClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this,           &SaveWorkspaceDialog::onDeleteClicked);

    resize(400, 380);
}

QString SaveWorkspaceDialog::startupWorkspaceName() const {
    if (!m_loadOnStartupCheck->isChecked())
        return {};
    // Если вводится новая — она и есть startup
    const QString newName = m_nameEdit->text().trimmed();
    if (!newName.isEmpty())
        return newName;
    // Иначе — выделенная в списке
    const QListWidgetItem* item = m_listWidget->currentItem();
    return item ? item->text() : QString{};
}

QString SaveWorkspaceDialog::newWorkspaceName() const
{
    return m_newWorkspaceName;
}

bool SaveWorkspaceDialog::loadOnStartup() const
{
    return m_loadOnStartupCheck->isChecked();
}

QStringList SaveWorkspaceDialog::deletedWorkspaceNames() const {
    return m_deletedNames;
}

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

void SaveWorkspaceDialog::onDeleteClicked() {
    QListWidgetItem* item = m_listWidget->currentItem();
    if (!item) return;

    const QString name = item->text();
    m_deletedNames.append(name);
    delete item; // удаляем из списка в диалоге
}

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
