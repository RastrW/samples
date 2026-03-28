#include "condFormatManager.h"
#include "condFormat.h"
#include "Settings.h"
#include "filterlineedit.h"

#include <QColorDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QMessageBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QToolButton>
#include <QTreeWidget>
#include <QApplication>
#include <QHeaderView>
#include <QLabel>

// Styled Item Delegate for non-editable columns (all except Condition)
class DefaultDelegate: public QStyledItemDelegate {
public:
    explicit DefaultDelegate(QObject* parent=nullptr): QStyledItemDelegate(parent) {}
    QWidget* createEditor(QWidget* /* parent */, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const override {
        return nullptr;
    }
};

// Styled Item Delegate for the Condition column as a filter line editor.
class FilterEditDelegate: public QStyledItemDelegate {

public:
    explicit FilterEditDelegate(QObject* parent=nullptr): QStyledItemDelegate(parent) {}
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const override {
        FilterLineEdit* filterEditor = new FilterLineEdit(parent);
        //filterEditor->setConditionFormatContextMenuEnabled(false);
        return filterEditor;
    }
};

CondFormatManager::CondFormatManager(const std::vector<CondFormat>& condFormats, const QString& encoding, QWidget *parent) :
    QDialog(parent),
    m_encoding(encoding)
{
    setupWidgets();

    for (const CondFormat& cf : condFormats)
        addItem(cf);
}

CondFormatManager::~CondFormatManager(){}

void CondFormatManager::setupWidgets()
{
    resize(750, 400);
    setWindowTitle(tr("Conditional Format Manager"));

    auto* rootLayout = new QVBoxLayout(this);

    // ── Описание ─────────────────────────────────────────────────────────────
    m_labelTitle = new QLabel(
        tr("This dialog allows creating and editing conditional formats. "
           "Each cell style will be selected by the first accomplished condition "
           "for that cell data. Conditional formats can be moved up and down, "
           "where those at higher rows take precedence over those at lower. "
           "Syntax for conditions is the same as for filters and an empty "
           "condition applies to all values."),
        this);
    m_labelTitle->setWordWrap(true);
    rootLayout->addWidget(m_labelTitle);

    // ── Панель кнопок управления строками ────────────────────────────────────
    auto* toolLayout = new QHBoxLayout;

    // Создать QToolButton с иконкой, текстом и подсказкой.
    auto makeButton = [this](const QString& iconPath,
                             const QString& text,
                             const QString& tooltip) -> QToolButton*
    {
        auto* btn = new QToolButton(this);
        btn->setIcon(QIcon(iconPath));
        btn->setText(text);
        btn->setToolTip(tooltip);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setAutoRaise(true);
        return btn;
    };

    m_buttonAdd    = makeButton(":/icons/field_add",    tr("&Add"),       tr("Add new conditional format"));
    m_buttonRemove = makeButton(":/icons/field_delete", tr("&Remove"),    tr("Remove selected conditional format"));
    m_buttonUp     = makeButton(":/icons/up",           tr("Move &up"),   tr("Move selected conditional format up"));
    m_buttonDown   = makeButton(":/icons/down",         tr("Move &down"), tr("Move selected conditional format down"));

    toolLayout->addWidget(m_buttonAdd);
    toolLayout->addWidget(m_buttonRemove);
    toolLayout->addWidget(m_buttonUp);
    toolLayout->addWidget(m_buttonDown);
    toolLayout->addStretch();
    rootLayout->addLayout(toolLayout);

    // ── Таблица условий ──────────────────────────────────────────────────────
    m_table = new QTreeWidget(this);

    // Заголовок: колонки с иконками (Bold/Italic/Underline) и текстовые.
    auto* header = new QTreeWidgetItem;
    header->setText(ColumnForeground, tr("Foreground"));
    header->setToolTip(ColumnForeground, tr("Text color"));
    header->setText(ColumnBackground, tr("Background"));
    header->setToolTip(ColumnBackground, tr("Background color"));
    header->setText(ColumnFont,       tr("Font"));
    header->setText(ColumnSize,       tr("Size"));
    // Колонки Bold/Italic/Underline — только иконки, текст пустой
    header->setIcon(ColumnBold,      QIcon(":/icons/text_bold"));
    header->setToolTip(ColumnBold,   tr("Bold"));
    header->setIcon(ColumnItalic,    QIcon(":/icons/text_italic"));
    header->setToolTip(ColumnItalic, tr("Italic"));
    header->setIcon(ColumnUnderline, QIcon(":/icons/text_underline"));
    header->setToolTip(ColumnUnderline, tr("Underline"));
    header->setText(ColumnAlignment, tr("Alignment"));
    header->setText(ColumnFilter,    tr("Condition"));
    m_table->setHeaderItem(header);

    // Настройки внешнего вида
    m_table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_table->setTabKeyNavigation(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_table->setIndentation(0);
    m_table->setRootIsDecorated(false);
    m_table->setUniformRowHeights(true);
    m_table->setItemsExpandable(false);
    m_table->setExpandsOnDoubleClick(false);
    m_table->header()->setMinimumSectionSize(25);

    // Делегаты: колонка Filter — FilterLineEdit, остальные — не редактируемые
    m_table->setItemDelegateForColumn(ColumnFilter, new FilterEditDelegate(this));
    for (int col = ColumnForeground; col < ColumnFilter; ++col) {
        m_table->setItemDelegateForColumn(col, new DefaultDelegate(this));
        m_table->resizeColumnToContents(col);
    }

    rootLayout->addWidget(m_table);

    // ── Кнопки Ok/Cancel/Help/Reset ──────────────────────────────────────────
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
            QDialogButtonBox::Help | QDialogButtonBox::Reset,
        Qt::Horizontal, this);
    rootLayout->addWidget(m_buttonBox);

    // ── Соединения ───────────────────────────────────────────────────────────
    connect(m_buttonAdd,    &QToolButton::clicked, this, &CondFormatManager::addNewItem);
    connect(m_buttonRemove, &QToolButton::clicked, this, &CondFormatManager::removeItem);
    connect(m_buttonUp,     &QToolButton::clicked, this, &CondFormatManager::upItem);
    connect(m_buttonDown,   &QToolButton::clicked, this, &CondFormatManager::downItem);
    connect(m_table, &QTreeWidget::itemClicked, this, &CondFormatManager::itemClicked);
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &CondFormatManager::buttonBoxClicked);
}

void CondFormatManager::addNewItem()
{
    // Сохранить текущий ввод в FilterLineEdit перед добавлением новой строки
    QWidget* current = m_table->itemWidget(m_table->currentItem(), ColumnFilter);
    if (current) current->clearFocus();

    QFont font = QFont(Settings::getValue("databrowser", "font").toString());
    font.setPointSize(Settings::getValue("databrowser", "fontsize").toInt());

    addItem(CondFormat(
        "",
        QColor(Settings::getValue("databrowser", "reg_fg_colour").toString()),
        m_condFormatPalette.nextSerialColor(Palette::appHasDarkTheme()),
        font,
        CondFormat::AlignLeft,
        m_encoding));
}

void CondFormatManager::addItem(const CondFormat& aCondFormat)
{
    const int i = m_table->topLevelItemCount();
    auto* item = new QTreeWidgetItem(m_table);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

    item->setForeground(ColumnForeground, aCondFormat.foregroundColor());
    item->setBackground(ColumnForeground, aCondFormat.foregroundColor());
    item->setForeground(ColumnBackground, aCondFormat.backgroundColor());
    item->setBackground(ColumnBackground, aCondFormat.backgroundColor());
    item->setToolTip(ColumnForeground, tr("Click to select color"));
    item->setToolTip(ColumnBackground, tr("Click to select color"));

    auto* fontCombo = new QFontComboBox(m_table);
    fontCombo->setCurrentFont(aCondFormat.font());
    fontCombo->setMaximumWidth(150);   // предотвращает перекрытие соседних колонок
    m_table->setItemWidget(item, ColumnFont, fontCombo);

    auto* sizeBox = new QSpinBox(m_table);
    sizeBox->setMinimum(1);
    sizeBox->setValue(aCondFormat.font().pointSize());
    m_table->setItemWidget(item, ColumnSize, sizeBox);

    item->setCheckState(ColumnBold,      aCondFormat.isBold()      ? Qt::Checked : Qt::Unchecked);
    item->setCheckState(ColumnItalic,    aCondFormat.isItalic()    ? Qt::Checked : Qt::Unchecked);
    item->setCheckState(ColumnUnderline, aCondFormat.isUnderline() ? Qt::Checked : Qt::Unchecked);

    auto* alignCombo = new QComboBox(m_table);
    alignCombo->addItems(CondFormat::alignmentTexts());
    alignCombo->setCurrentIndex(aCondFormat.alignment());
    m_table->setItemWidget(item, ColumnAlignment, alignCombo);

    item->setText(ColumnFilter, aCondFormat.filter());
    m_table->insertTopLevelItem(i, item);

    for (int col = ColumnForeground; col < ColumnFilter; ++col)
        m_table->resizeColumnToContents(col);
}

void CondFormatManager::removeItem()
{
    delete m_table->takeTopLevelItem(m_table->currentIndex().row());
}

void CondFormatManager::moveItem(int offset)
{
    if (!m_table->currentIndex().isValid()) return;

    const int selectedRow = m_table->currentIndex().row();
    const int newRow      = selectedRow + offset;
    if (newRow < 0 || newRow >= m_table->topLevelItemCount()) return;

    QTreeWidgetItem* item = m_table->topLevelItem(selectedRow);

    // Qt удаляет дочерние виджеты при takeTopLevelItem,
    // поэтому перед извлечением создаём копии виджетов.
    auto rescueFont = [&]() {
        auto* src = qobject_cast<QFontComboBox*>(m_table->itemWidget(item, ColumnFont));
        auto* dst = new QFontComboBox(m_table);
        dst->setCurrentFont(src->currentFont());
        dst->setMaximumWidth(150);
        return dst;
    };
    auto rescueSize = [&]() {
        auto* src = qobject_cast<QSpinBox*>(m_table->itemWidget(item, ColumnSize));
        auto* dst = new QSpinBox(m_table);
        dst->setMinimum(src->minimum());
        dst->setValue(src->value());
        return dst;
    };
    auto rescueAlign = [&]() {
        auto* src = qobject_cast<QComboBox*>(m_table->itemWidget(item, ColumnAlignment));
        auto* dst = new QComboBox(m_table);
        dst->addItems(CondFormat::alignmentTexts());
        dst->setCurrentIndex(src->currentIndex());
        return dst;
    };

    QFontComboBox* newFont  = rescueFont();
    QSpinBox*      newSize  = rescueSize();
    QComboBox*     newAlign = rescueAlign();

    item = m_table->takeTopLevelItem(selectedRow);
    m_table->insertTopLevelItem(newRow, item);

    m_table->setItemWidget(item, ColumnFont,      newFont);
    m_table->setItemWidget(item, ColumnSize,      newSize);
    m_table->setItemWidget(item, ColumnAlignment, newAlign);

    m_table->setCurrentIndex(
        m_table->currentIndex().sibling(newRow, m_table->currentIndex().column()));
    m_table->adjustSize();
}

void CondFormatManager::upItem()   { moveItem(-1); }
void CondFormatManager::downItem() { moveItem(+1); }

std::vector<CondFormat> CondFormatManager::getCondFormats() const
{
    std::vector<CondFormat> result;
    result.reserve(m_table->topLevelItemCount());

    for (int i = 0; i < m_table->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_table->topLevelItem(i);

        auto* fontCombo  = qobject_cast<QFontComboBox*>(m_table->itemWidget(item, ColumnFont));
        auto* sizeBox    = qobject_cast<QSpinBox*>     (m_table->itemWidget(item, ColumnSize));
        auto* alignCombo = qobject_cast<QComboBox*>    (m_table->itemWidget(item, ColumnAlignment));

        QFont font = fontCombo->currentFont();
        font.setPointSize(sizeBox->value());
        font.setBold     (item->checkState(ColumnBold)      == Qt::Checked);
        font.setItalic   (item->checkState(ColumnItalic)    == Qt::Checked);
        font.setUnderline(item->checkState(ColumnUnderline) == Qt::Checked);

        result.emplace_back(
            item->text(ColumnFilter),
            item->background(ColumnForeground).color(),
            item->background(ColumnBackground).color(),
            font,
            static_cast<CondFormat::Alignment>(alignCombo->currentIndex()),
            m_encoding);
    }
    return result;
}

void CondFormatManager::itemClicked(QTreeWidgetItem* item, int column)
{
    switch (column) {
    case ColumnForeground:
    case ColumnBackground: {
        QColor color = QColorDialog::getColor(item->background(column).color(), this);
        if (color.isValid()) {
            item->setForeground(column, color);
            item->setBackground(column, color);
        }
        break;
    }
    case ColumnBold:
    case ColumnItalic:
    case ColumnUnderline:
        // Переключаем чекбокс вручную — QTreeWidget не делает это автоматически
        // при клике вне области чекбокса.
        item->setCheckState(column,
                            item->checkState(column) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        break;
    default:
        break;
    }
}

void CondFormatManager::buttonBoxClicked(QAbstractButton* button)
{
    const auto role = m_buttonBox->buttonRole(button);

    if (role == QDialogButtonBox::AcceptRole) {
        accept();
    } else if (role == QDialogButtonBox::RejectRole) {
        reject();
    } else if (role == QDialogButtonBox::HelpRole) {
        QDesktopServices::openUrl(
            QUrl("https://github.com/sqlitebrowser/sqlitebrowser/wiki/Conditional-Formats"));
    } else if (role == QDialogButtonBox::ResetRole) {
        // сравнение через buttonRole() — не зависит от конкретного указателя
        const auto answer = QMessageBox::warning(
            this,
            QApplication::applicationName(),
            tr("Are you sure you want to clear all the conditional formats of this field?"),
            QMessageBox::Reset | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (answer == QMessageBox::Reset)
            m_table->clear();   // заменяет removeRows(0, rowCount()) — чище
    }
}
