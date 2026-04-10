#include "filterCell.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QMenu>
#include <QDoubleValidator>

FilterCell::FilterCell(bool isNumeric, bool isBool, QWidget* parent)
    : QWidget(parent)
    , m_isNumeric(isNumeric)
    , m_isBool(isBool)
{
    setFixedHeight(kRowHeight);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    auto* lay = new QHBoxLayout(this);
    // Убрали лишние отступы: был (1, 0, 1, 0) → (0, 0, 0, 0)
    lay->setContentsMargins(0, 0, 0, 0);
    // Убрали spacing между кнопкой и полем: был 2 → 0
    lay->setSpacing(0);

    m_opBtn = new QToolButton(this);
    m_opBtn->setCursor(Qt::PointingHandCursor);
    m_opBtn->setAutoRaise(false);
    m_opBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_opBtn->setFixedHeight(kRowHeight - 2);
    // Убрали жёсткую ширину — будет устанавливаться динамически
    connect(m_opBtn, &QToolButton::clicked, this, &FilterCell::showOpMenu);
    lay->addWidget(m_opBtn, 0);

    if (!m_isBool) {
        m_edit = new QLineEdit(this);
        m_edit->setFrame(false);
        m_edit->setFixedHeight(kRowHeight - 2);
        m_edit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        if (m_isNumeric) {
            auto* dv = new QDoubleValidator(m_edit);
            dv->setNotation(QDoubleValidator::StandardNotation);
            m_edit->setValidator(dv);
        }

        connect(m_edit, &QLineEdit::textChanged,
                this, &FilterCell::slotTextChanged);

        lay->addWidget(m_edit, 1);
    }

    // Улучшенная стилизация: кнопка и поле теперь одного стиля
    setStyleSheet(
        "QToolButton {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 2px;"
        "  padding: 0 3px;"
        "  background: palette(base);"
        "  margin: 1px;"
        "}"
        "QToolButton:hover {"
        "  border: 1px solid palette(highlight);"
        "}"
        "QLineEdit {"
        "  border: 1px solid palette(mid);"      // Добавили видимую границу
        "  border-radius: 2px;"
        "  background: palette(base);"
        "  padding: 0 3px;"
        "  margin: 1px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid palette(highlight);"
        "}"
        );

    if (!m_isBool) {
        m_rule.op = FilterRule::Op::Eq;
    }
    updateUi();
}

FilterRule FilterCell::rule() const { return m_rule; }

void FilterCell::setRule(const FilterRule& rule)
{
    m_rule = rule;

    if (m_edit) {
        QSignalBlocker blk(m_edit);
        m_edit->setText(rule.value);
    }

    updateUi();
}

void FilterCell::clear()
{
    m_rule = FilterRule{};
    if (m_edit) {
        QSignalBlocker blk(m_edit);
        m_edit->clear();
    }
    updateUi();
    emit sig_filterChanged(m_rule);
}

QSize FilterCell::minimumSizeHint() const
{
    // Минимальный размер: кнопка + хоть что-то на поле
    const int btnW = m_isBool ? BOOL_BTN_WIDTH : MIN_BTN_WIDTH;
    const int editW = m_isBool ? 0 : 15;

    // Без лишних отступов (убрали margins 2+2)
    return QSize(btnW + editW, kRowHeight);
}

QSize FilterCell::sizeHint() const
{
    // Предпочтительный размер
    const int btnW = m_isBool ? BOOL_BTN_WIDTH : MIN_BTN_WIDTH;
    const int editW = m_isBool ? 0 : 40;

    return QSize(btnW + editW, kRowHeight);
}

void FilterCell::setPlaceholderText(const QString& text)
{
    m_placeholder = text;
    if (m_edit && m_edit->placeholderText() != text)
        m_edit->setPlaceholderText(text);
}

QString FilterCell::opText(FilterRule::Op op) const
{
    switch (op) {
    case FilterRule::Op::Eq:        return QStringLiteral("=");
    case FilterRule::Op::Neq:       return QStringLiteral("≠");
    case FilterRule::Op::Lt:        return QStringLiteral("<");
    case FilterRule::Op::Le:        return QStringLiteral("≤");
    case FilterRule::Op::Gt:        return QStringLiteral(">");
    case FilterRule::Op::Ge:        return QStringLiteral("≥");
    case FilterRule::Op::Contains:   return QStringLiteral("⊃");
    case FilterRule::Op::Like:      return QStringLiteral("~");
    case FilterRule::Op::NotLike:   return QStringLiteral("¬~");
    case FilterRule::Op::StartsWith:return QStringLiteral("^");
    case FilterRule::Op::EndsWith:  return QStringLiteral("$");
    case FilterRule::Op::None:      return QStringLiteral("⋯");
    }
    return QStringLiteral("⋯");
}

void FilterCell::updateUi()
{
    if (!m_opBtn)
        return;

    if (m_isBool) {
        if (!m_rule.isActive()) {
            m_opBtn->setText(QStringLiteral("bool"));
        }
        else {
            m_opBtn->setText(m_rule.boolValue ? QStringLiteral("true")
                                              : QStringLiteral("false"));
        }
    }
    else {
        m_opBtn->setText(opText(m_rule.op));
        if (m_edit && m_edit->placeholderText() != m_placeholder)
            m_edit->setPlaceholderText(m_placeholder);
    }
}

void FilterCell::applyRule(const FilterRule& rule, bool emitSignal)
{
    m_rule = rule;

    if (m_isBool) {
        m_rule.isBool = true;
        m_rule.isNum  = false;
        if (m_rule.op == FilterRule::Op::None) {
            m_rule.boolValue = false;
            m_rule.value.clear();
        }
        else {
            m_rule.value = m_rule.boolValue ? QStringLiteral("1") : QStringLiteral("0");
        }
    }
    else {
        m_rule.isBool = false;

        if (m_edit) {
            QSignalBlocker blk(m_edit);
            m_edit->setText(m_rule.value);
        }
    }

    updateUi();

    if (emitSignal)
        emit sig_filterChanged(m_rule);
}

void FilterCell::slotTextChanged(const QString& text)
{
    if (m_isBool)
        return;

    FilterRule rule = m_rule;
    rule.value = text;
    rule.isBool = false;
    rule.isNum = false;

    const QString trimmed = text.trimmed();

    if (trimmed.isEmpty()) {
        rule = FilterRule{};
        m_rule = rule;
        updateUi();
        emit sig_filterChanged(rule);
        return;
    }

    if (rule.op == FilterRule::Op::None)
        rule.op = FilterRule::Op::Eq;

    if (m_isNumeric) {
        bool ok = false;
        const double v = trimmed.toDouble(&ok);
        if (ok) {
            rule.isNum = true;
            rule.numValue = v;
        }
    }

    m_rule = rule;
    updateUi();
    emit sig_filterChanged(m_rule);
}

void FilterCell::showOpMenu()
{
    QMenu menu(this);

    auto emitCurrent = [this]() {
        updateUi();
        emit sig_filterChanged(m_rule);
    };

    auto addOp = [&](const QString& label, FilterRule::Op op) {
        QAction* act = menu.addAction(label);
        act->setCheckable(true);
        act->setChecked(m_rule.op == op);

        connect(act, &QAction::triggered, this, [this, op, emitCurrent]() {
            if (m_isBool) {
                return;
            }

            m_rule.op = op;
            m_rule.isBool = false;

            if (m_edit) {
                m_rule.value = m_edit->text();
            }

            if (m_isNumeric) {
                bool ok = false;
                const QString trimmed = m_edit ? m_edit->text().trimmed() : QString();
                const double v = trimmed.toDouble(&ok);
                m_rule.isNum = ok;
                if (ok)
                    m_rule.numValue = v;
            } else {
                m_rule.isNum = false;
            }

            emitCurrent();
        });
    };

    auto addBool = [&](const QString& label, bool value) {
        QAction* act = menu.addAction(label);
        act->setCheckable(true);
        act->setChecked(m_rule.isActive() && m_rule.isBool && m_rule.boolValue == value);

        connect(act, &QAction::triggered, this, [this, value]() {
            m_rule = FilterRule{};
            m_rule.op = FilterRule::Op::Eq;
            m_rule.isBool = true;
            m_rule.boolValue = value;
            m_rule.value = value ? QStringLiteral("1") : QStringLiteral("0");

            updateUi();
            emit sig_filterChanged(m_rule);
        });
    };

    if (m_isBool) {
        addBool(QStringLiteral("true"), true);
        addBool(QStringLiteral("false"), false);
    } else {
        addOp(QStringLiteral("Равно"), FilterRule::Op::Eq);
        addOp(QStringLiteral("Не равно"), FilterRule::Op::Neq);
        addOp(QStringLiteral("Больше"), FilterRule::Op::Gt);
        addOp(QStringLiteral("Больше или равно"), FilterRule::Op::Ge);
        addOp(QStringLiteral("Меньше"), FilterRule::Op::Lt);
        addOp(QStringLiteral("Меньше или равно"), FilterRule::Op::Le);

        if (!m_isNumeric) {
            menu.addSeparator();
            addOp(QStringLiteral("Содержит"), FilterRule::Op::Contains);
            addOp(QStringLiteral("Соответствует маске"), FilterRule::Op::Like);
            addOp(QStringLiteral("Не соответствует маске"), FilterRule::Op::NotLike);
            addOp(QStringLiteral("Начинается с"), FilterRule::Op::StartsWith);
            addOp(QStringLiteral("Заканчивается с"), FilterRule::Op::EndsWith);
        }
    }

    menu.addSeparator();

    QAction* clearAct = menu.addAction(QStringLiteral("Сбросить"));
    connect(clearAct, &QAction::triggered, this, [this]() {
        clear();
    });

    menu.exec(m_opBtn->mapToGlobal(QPoint(0, m_opBtn->height())));
}

void FilterCell::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateButtonWidth();
}

void FilterCell::updateButtonWidth()
{
    if (!m_opBtn)
        return;

    int availableWidth = width();
    int btnWidth;

    if (m_isBool) {
        // Для bool кнопка занимает всю ширину, но не менее MIN_BTN_WIDTH
        btnWidth = qMax(BOOL_BTN_WIDTH, availableWidth);
    } else {
        // Для обычных полей:
        // - если много места, кнопка MIN_BTN_WIDTH
        // - если мало места (узкая колонка), кнопка адаптируется, но не менее MIN_BTN_WIDTH
        const int minTotalWidth = MIN_BTN_WIDTH + 10; // минимум на QLineEdit

        if (availableWidth >= minTotalWidth + 40) {
            // Много места → маленькая кнопка, большое поле
            btnWidth = MIN_BTN_WIDTH;
        } else if (availableWidth >= minTotalWidth) {
            // Среднее место → адаптивная кнопка
            btnWidth = qMax(MIN_BTN_WIDTH, availableWidth / 4);
        } else {
            // Мало места → кнопка займёт примерно 50% (но не менее MIN)
            btnWidth = qMax(MIN_BTN_WIDTH, availableWidth / 2 - 2);
        }
    }

    m_opBtn->setFixedWidth(btnWidth);
}

