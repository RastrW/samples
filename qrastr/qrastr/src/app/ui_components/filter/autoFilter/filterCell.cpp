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
    lay->setContentsMargins(1, 0, 1, 0);
    lay->setSpacing(2);

    m_opBtn = new QToolButton(this);
    m_opBtn->setCursor(Qt::PointingHandCursor);
    m_opBtn->setAutoRaise(false);
    m_opBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_opBtn->setFixedHeight(kRowHeight - 2);
    m_opBtn->setFixedWidth(m_isBool ? 74 : 30);
    connect(m_opBtn, &QToolButton::clicked, this, &FilterCell::showOpMenu);
    lay->addWidget(m_opBtn, 0);

    if (!m_isBool) {
        m_edit = new QLineEdit(this);
        m_edit->setFrame(false);
        m_edit->setFixedHeight(kRowHeight - 4);
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

    setStyleSheet(
        "QToolButton {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 2px;"
        "  padding: 0 4px;"
        "  background: palette(base);"
        "}"
        "QToolButton:hover {"
        "  border: 1px solid palette(highlight);"
        "}"
        "QLineEdit {"
        "  border: 0px;"
        "  background: transparent;"
        "  padding: 0 3px;"
        "}"
        );
    // По умолчанию — '=' для не-bool, пусто для bool.
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
    const int margins = 2 + 2;   // left + right
    const int spacing = 2;

    const int btnW = m_isBool ? 74 : 30;
    const int editW = m_isBool ? 0 : 20;

    return QSize(margins + btnW + (m_isBool ? 0 : spacing + editW) + margins,
                 kRowHeight);
}

QSize FilterCell::sizeHint() const
{
    return minimumSizeHint();
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
            // пустое состояние
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

    // Пусто → сброс условия
    if (trimmed.isEmpty()) {
        rule = FilterRule{};
        m_rule = rule;
        updateUi();
        emit sig_filterChanged(rule);
        return;
    }

    // Если оператор ещё не выбран, считаем, что это "="
    if (rule.op == FilterRule::Op::None)
        rule.op = FilterRule::Op::Eq;

    // Для чисел пытаемся распарсить значение
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
