#include "saveworkspacedialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QTreeWidget>
#include <QButtonGroup>
#include <QHeaderView>
#include <QRadioButton>
#include <QAbstractButton>

SaveWorkspaceDialog::SaveWorkspaceDialog(const QStringList &workspaces,
                                         const QString     &startupName,
                                         QWidget           *parent)
    : WorkspaceDialogBase(workspaces, parent)
    , m_radioGroup(new QButtonGroup(this))
    , m_deleteButton(new QPushButton(tr("Удалить"), this))
{
    setWindowTitle(tr("Сохранить рабочую область"));

    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels({tr("Рабочая область"), tr("При старте")});
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_tree->header()->resizeSection(1, 90);

    setupTree(startupName);
    setupAddRow();   // ← внутри добавляется m_noStartupRadio и insertWidgetBelowList

    insertWidgetBelowList(m_addRow);
    insertWidgetBelowList(m_deleteButton);

    finalizeLayout();

    // Если startup не задан — явно отмечаем «Не загружать»
    if (startupWorkspaceName().isEmpty())
        m_noStartupRadio->setChecked(true);

    disconnect(m_buttonBox, &QDialogButtonBox::accepted,
               this,        &QDialog::accept);
    connect(m_buttonBox,    &QDialogButtonBox::accepted,
            this,           &SaveWorkspaceDialog::onOkClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this,           &SaveWorkspaceDialog::onDeleteClicked);

    resize(440, 420);
}

void SaveWorkspaceDialog::setupTree(const QString &startupName)
{
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_tree->topLevelItem(i);

        auto *radio = new QRadioButton(this);
        m_tree->setItemWidget(item, 1, radio);
        m_radioGroup->addButton(radio);

        if (item->text(0) == startupName)
            radio->setChecked(true);
    }
}

void SaveWorkspaceDialog::setupAddRow()
{
    // Кнопка «Не загружать при старте» — добавляем ДО строки ввода,
    // чтобы она располагалась над ней в layout
    m_noStartupRadio = new QRadioButton(tr("Не загружать при старте"), this);
    m_radioGroup->addButton(m_noStartupRadio);   // входит в ту же группу!
    insertWidgetBelowList(m_noStartupRadio);      // вызвать до finalizeLayout

    m_addRow     = new QWidget(this);
    m_nameEdit   = new QLineEdit(m_addRow);
    m_nameEdit->setPlaceholderText(tr("Введите название…"));
    m_nameEdit->setVisible(false);

    m_addButton    = new QPushButton(tr("Добавить"), m_addRow);
    m_addOkBtn     = new QPushButton(tr("Ok"),       m_addRow);
    m_addCancelBtn = new QPushButton(tr("Отмена"),   m_addRow);
    m_addOkBtn->setVisible(false);
    m_addCancelBtn->setVisible(false);

    auto *layout = new QHBoxLayout(m_addRow);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_addButton);
    layout->addWidget(m_nameEdit,   1);
    layout->addWidget(m_addOkBtn);
    layout->addWidget(m_addCancelBtn);

    connect(m_addButton,    &QPushButton::clicked,
            this,           &SaveWorkspaceDialog::onAddButtonClicked);
    connect(m_addOkBtn,     &QPushButton::clicked,
            this,           &SaveWorkspaceDialog::onAddOkClicked);
    connect(m_addCancelBtn, &QPushButton::clicked,
            this,           &SaveWorkspaceDialog::onAddCancelClicked);
    connect(m_nameEdit,     &QLineEdit::returnPressed,
            this,           &SaveWorkspaceDialog::onAddOkClicked);
}

void SaveWorkspaceDialog::updateStartupControls()
{
    const bool hasItems = m_tree->topLevelItemCount() > 0;

    // Если список опустел — переключаем на «Не загружать»
    if (!hasItems)
        m_noStartupRadio->setChecked(true);

    // Радиокнопки в дереве блокируем только визуально при пустом списке
    // (сама QButtonGroup exclusive не даст двух отмеченных)
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        if (auto *w = m_tree->itemWidget(m_tree->topLevelItem(i), 1))
            w->setEnabled(true);
    }
}

void SaveWorkspaceDialog::onAddButtonClicked()
{
    const bool show = !m_nameEdit->isVisible();
    m_nameEdit->setVisible(show);
    m_addOkBtn->setVisible(show);
    m_addCancelBtn->setVisible(show);
    // прячем «Добавить» пока идёт ввод
    m_addButton->setVisible(!show);

    if (show)
        m_nameEdit->setFocus();
}

void SaveWorkspaceDialog::onAddOkClicked()
{
    const QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) { m_nameEdit->setFocus(); return; }

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        if (m_tree->topLevelItem(i)->text(0).compare(name, Qt::CaseInsensitive) == 0) {
            QMessageBox::warning(this, tr("Дублирование"),
                                 tr("Рабочая область «%1» уже существует.").arg(name));
            m_nameEdit->selectAll();
            m_nameEdit->setFocus();
            return;
        }
    }

    auto *item = new QTreeWidgetItem(m_tree);
    item->setText(0, name);

    auto *radio = new QRadioButton(this);
    m_tree->setItemWidget(item, 1, radio);
    m_radioGroup->addButton(radio);
    m_tree->setCurrentItem(item);

    if (m_tree->topLevelItemCount() == 1)
        radio->setChecked(true);

    m_newWorkspaceName = name;

    // Блокируем повторное добавление
    m_addButton->setEnabled(false);

    updateStartupControls();
    onAddCancelClicked();
}

void SaveWorkspaceDialog::onAddCancelClicked()
{
    m_nameEdit->clear();
    m_nameEdit->setVisible(false);
    m_addOkBtn->setVisible(false);
    m_addCancelBtn->setVisible(false);
    m_addButton->setVisible(true);
}

void SaveWorkspaceDialog::onDeleteClicked()
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item) return;

    const QString name = item->text(0);
    m_deletedNames.append(name);

    // Если удаляем только что добавленную область — разрешаем добавить снова
    if (name == m_newWorkspaceName) {
        m_newWorkspaceName.clear();
        m_addButton->setEnabled(true);
    }

    delete item;
    updateStartupControls();
}

void SaveWorkspaceDialog::onOkClicked()
{
    // Если строка добавления открыта и не пуста — считаем это
    // незавершённым вводом; предупреждаем пользователя
    if (m_nameEdit->isVisible() && !m_nameEdit->text().trimmed().isEmpty()) {
        const auto btn = QMessageBox::question(
            this, tr("Незавершённый ввод"),
            tr("Вы ввели название новой области, но не нажали «Ok».\n"
               "Добавить «%1» перед сохранением?")
                .arg(m_nameEdit->text().trimmed()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (btn == QMessageBox::Cancel) return;
        // остаёмся в диалоге
        if (btn == QMessageBox::Yes)   { onAddOkClicked(); return; }
    }

    accept();
}

QString SaveWorkspaceDialog::newWorkspaceName() const
{
    return m_newWorkspaceName;
}

QString SaveWorkspaceDialog::startupWorkspaceName() const
{
    // m_noStartupRadio тоже в группе, но не привязан к строке дерева —
    // цикл просто не найдёт для него совпадения и вернёт пустую строку.
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto *w = qobject_cast<QRadioButton*>(
            m_tree->itemWidget(m_tree->topLevelItem(i), 1));
        if (w && w->isChecked())
            return m_tree->topLevelItem(i)->text(0);
    }
    // m_noStartupRadio отмечен → возвращаем пустую строку → startup сбрасывается
    return {};
}

QStringList SaveWorkspaceDialog::deletedWorkspaceNames() const
{
    return m_deletedNames;
}