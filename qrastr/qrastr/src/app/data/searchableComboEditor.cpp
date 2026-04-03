#include "searchableComboEditor.h"
#include "searchableComboRepository.h"

void SearchableComboEditor::createEditModeContext()
{
    m_selectedIdx = -1;  // сброс при каждом открытии

    const auto* repo = static_cast<SearchableComboRepository*>(editorRepository());
    int idx = getContextValue().toInt();
    m_current = (idx >= 0 && idx < repo->items().size())
                    ? repo->items().at(idx)
                    : QString{};
    showPopup();
}

void SearchableComboEditor::destroyEditModeContext()
{
    // возвращаем поведение по умолчанию
    editorRepository()->setImmediatePost(true);
    if (m_popup) {
        m_popup->hide();
        m_popup->deleteLater();
        m_popup = nullptr;
    }
}

QVariant SearchableComboEditor::getContextValue() const
{
    if (m_selectedIdx >= 0) {
        const auto* repo = static_cast<SearchableComboRepository*>(editorRepository());
        if (m_selectedIdx < repo->items().size())
            return repo->items().at(m_selectedIdx);  // ← строка, не индекс
    }
    return Qtitan::GridEditorBase::getContextValue();
}

bool SearchableComboEditor::isContextModified()
{
    return m_selectedIdx >= 0
               ? true
               : Qtitan::GridEditorBase::isContextModified();
}

void SearchableComboEditor::setValueToWidget(const QVariant& value)
{
    const auto* repo = static_cast<SearchableComboRepository*>(editorRepository());
    int idx = value.toInt();
    if (idx >= 0 && idx < repo->items().size())
        m_current = repo->items().at(idx);
    else
        m_current = value.toString();

    if (m_popup)
        m_popup->setCurrentText(m_current);
}

void SearchableComboEditor::showPopup()
{
    auto* repo       = static_cast<SearchableComboRepository*>(editorRepository());
    auto* gridWidget = repo->gridWidget();
    if (!gridWidget) return;

    editorRepository()->setImmediatePost(false);

    m_popup = new SearchableComboPopup(gridWidget->window());
    m_popup->setItems(repo->items());
    m_popup->setCurrentText(m_current);

    const QPoint cursorPos = QCursor::pos();
    m_popup->move(cursorPos.x(), cursorPos.y() + 4);
    m_popup->setFixedWidth(qMax(site()->geometry().width(), 250));
    m_popup->show();
    m_popup->setFocus();

    QObject::connect(m_popup, &SearchableComboPopup::itemSelected,
                     [this, repo](const QString& text) {
                         // Сохраняем ДО того как попап закрывается
                         // и QTitan начинает читать getContextValue()
                         m_selectedIdx = repo->items().indexOf(text);
                         setContextModified(true);

                         // postEditor читает getContextValue() — там уже m_selectedIdx
                         editorRepository()->view()->postEditor();
                     });

    QObject::connect(m_popup, &SearchableComboPopup::editingCancelled,
                     [this]() {
                         m_selectedIdx = -1;
                         editorRepository()->view()->hideEditor();
                     });
}