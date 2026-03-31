#include "selectionDialog.h"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QEvent>

SelectionDialog::SelectionDialog(std::string selection,
                                 std::string colName,
                                 QWidget*    parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Выборка"));
    setWindowModality(Qt::ApplicationModal);
    setSizeGripEnabled(false);

    const QString col = QString::fromStdString(colName);


    // ── Строка ввода ─────────────────────────────────────────────────
    m_edit = new QLineEdit(this);
    m_edit->setMinimumWidth(360);
    m_edit->setFont(QFont("Consolas, Courier New", 10));
    if (!selection.empty())
        m_edit->setText(QString::fromStdString(selection));
    else
        m_edit->setPlaceholderText(
            col.isEmpty() ? "pn>100&tip!=2" : col + ">10");

    // ── Кнопки быстрой вставки ───────────────────────────────────────
    struct Btn { QString label; QString insert; QString tip; };
    const Btn btns[] = {
                        { ">",   ">",   tr("больше")            },
                        { "<",   "<",   tr("меньше")            },
                        { "=",   "=",   tr("равно")             },
                        { "!=",  "!=",  tr("не равно")          },
                        { "~",   "~",   tr("приблизительно")    },
//"&" → воспринимается как начало указания горячей клавиши,поэтому требуется указывать два операнда
                        { "&&",  "&",   tr("И")                 },
                        { "|",   "|",   tr("ИЛИ")               },
                        { "!",   "!",   tr("НЕ")                },
                        { "( )", "()",  tr("скобки")            },
                        };

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(3);
    for (const auto& b : btns) {
        auto* btn = new QPushButton(b.label, this);
        btn->setToolTip(b.tip);
        btn->setFixedSize(40, 24);
        btn->setFocusPolicy(Qt::NoFocus);   // фокус остаётся на m_edit
        const QString ins = b.insert;
        connect(btn, &QPushButton::clicked, this, [this, ins]() {
            insertAtCursor(ins);
        });
        btnLayout->addWidget(btn);
    }
    btnLayout->addStretch();

    // ── Кнопки диалога ───────────────────────────────────────────────
    auto* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    auto* resetBtn = buttonBox->addButton(tr("Сбросить"), QDialogButtonBox::ResetRole);
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        emit sig_selectionAccepted("");
        close();
    });
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        emit sig_selectionAccepted(m_edit->text().trimmed().toStdString());
        close();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ── Компоновка ───────────────────────────────────────────────────
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_edit);
    layout->addLayout(btnLayout);
    layout->addWidget(buttonBox);

    m_edit->setFocus();
    m_edit->selectAll();   // сразу выделяем старую выборку — удобно перезаписать
}

void SelectionDialog::insertAtCursor(const QString& text)
{
    // Для "()" ставим курсор между скобками
    if (text == "()") {
        const int pos = m_edit->cursorPosition();
        m_edit->insert("()");
        m_edit->setCursorPosition(pos + 1);
    } else {
        m_edit->insert(text);
    }
    m_edit->setFocus();
}
