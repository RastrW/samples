#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include "dlgfindrepl.h"


DlgFindRepl::DlgFindRepl(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Find"));
    // Убираем кнопку «?» в заголовке
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_leFind = new QLineEdit(this);
    m_leFind->setPlaceholderText(tr("Search..."));
    m_leFind->setMinimumWidth(200);

    m_pbFind = new QPushButton(tr("Find"), this);
    m_pbFind->setDefault(true);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(m_leFind);
    layout->addWidget(m_pbFind);
    setLayout(layout);
    adjustSize();

    connect(m_pbFind, &QPushButton::clicked,
            this, &DlgFindRepl::slot_findClicked);

    // Enter в строке поиска тоже запускает поиск
    connect(m_leFind, &QLineEdit::returnPressed,
            this, &DlgFindRepl::slot_findClicked);
}

void DlgFindRepl::slot_findClicked()
{
    const QString text = m_leFind->text().trimmed();
    if (!text.isEmpty())
        emit sig_find(SciPyEditor::FindParams{text});
}
