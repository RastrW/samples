#include "menuSearchWidget.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>

MenuSearchWidget::MenuSearchWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(0);

    // ── Строка поиска ─────────────────────────────────────────────────────────
    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText(tr("Поиск действия..."));
    m_edit->setFixedWidth(200);
    m_edit->setClearButtonEnabled(true);
    layout->addWidget(m_edit);

    // ── Popup ─────────────────────────────────────────────────────────────────
    // Qt::Tool + WindowDoesNotAcceptFocus — окно видно, но НЕ крадёт фокус
    m_popup = new QListWidget(nullptr);
    m_popup->setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowDoesNotAcceptFocus);
    m_popup->setFocusPolicy(Qt::NoFocus);
    m_popup->setSelectionMode(QAbstractItemView::SingleSelection);
    m_popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_popup->hide();

    // ── Соединения ────────────────────────────────────────────────────────────
    connect(m_edit, &QLineEdit::textChanged,
            this,   &MenuSearchWidget::onTextChanged);
    connect(m_edit, &QLineEdit::returnPressed,
            this,   &MenuSearchWidget::onReturnPressed);
    connect(m_popup, &QListWidget::itemClicked,
            this,    &MenuSearchWidget::onItemActivated);

    // Скрываем popup при потере фокуса строкой поиска
    connect(m_edit, &QLineEdit::textChanged, this, [this](const QString& t) {
        if (t.isEmpty()) hidePopup();   //popup закрывается по пустому тексту
    });
}

void MenuSearchWidget::setActions(const std::map<QString, QAction*>& actions)
{
    for (auto& [key, action] : actions)
        registerAction(action);
}

void MenuSearchWidget::addActionsFromMenu(QMenu* menu) // [4]
{
    if (!menu) return;
    for (QAction* action : menu->actions()) {
        if (action->isSeparator()) continue;
        if (action->menu()) {
            addActionsFromMenu(action->menu()); // рекурсия в подменю
        } else {
            registerAction(action);
        }
    }
}

void MenuSearchWidget::onTextChanged(const QString& text)
{
    m_popup->clear();

    const QString filter = text.trimmed();
    if (filter.isEmpty()) {
        hidePopup(); // основной путь закрытия
        return;
    }

    for (auto it = m_actionMap.constBegin(); it != m_actionMap.constEnd(); ++it) {
        QAction* action = it.value();
        if (!action) continue;

        if (it.key().startsWith(filter, Qt::CaseInsensitive)) {
            auto* item = new QListWidgetItem(m_popup);
            if (!action->icon().isNull())
                item->setIcon(action->icon());
            item->setText(it.key());

            if (!action->isEnabled()) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                item->setForeground(Qt::gray);
            }
            item->setData(Qt::UserRole, QVariant::fromValue<QAction*>(action));
        }
    }

    if (m_popup->count() > 0) {
        m_popup->setCurrentRow(0);
        showPopup();
    } else {
        hidePopup();
    }
}

void MenuSearchWidget::onItemActivated(QListWidgetItem* item){
    if (!item) return;
    auto* action = item->data(Qt::UserRole).value<QAction*>();
    hidePopup();
    m_edit->clear();
    if (action && action->isEnabled())
        action->trigger();
}

void MenuSearchWidget::onReturnPressed(){
    onItemActivated(m_popup->currentItem());
}

void MenuSearchWidget::registerAction(QAction* action){
    if (!action || action->isSeparator()) return;

    // Убираем мнемонику '&' и стили пропускаем
    const QString text = action->text().remove('&').trimmed();
    if (text.isEmpty()) return;
    if (action->objectName().startsWith("style_")) return;

    m_actionMap.insert(text, action);
}

void MenuSearchWidget::showPopup()
{
    repositionPopup();
    const int rowH  = m_popup->sizeHintForRow(0);
    const int count = qMin(m_popup->count(), 8);
    m_popup->setFixedSize(
        qMax(m_edit->width(), 220),
        rowH * count + m_popup->frameWidth() * 2);
    m_popup->show();
}

void MenuSearchWidget::hidePopup()
{
    m_popup->hide();
}

void MenuSearchWidget::repositionPopup()
{
    m_popup->move(m_edit->mapToGlobal(QPoint(0, m_edit->height())));
}