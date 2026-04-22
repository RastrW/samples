#include "searchableComboEditorTwo.h"
#include "searchableComboRepositoryTwo.h"
#include "searchableComboPopupTwo.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QLineEdit>
#include <QToolButton>

void SearchableComboEditorTwo::createEditModeContext()
{
    QWidget* parentWidget = site() ? site()->parent() : nullptr;

    m_container = new QWidget(parentWidget);
    auto* layout = new QHBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_lineEdit = new QLineEdit(m_container);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setFrame(false);

    m_button = new QToolButton(m_container);
    m_button->setText("...");
    m_button->setFixedWidth(20);

    layout->addWidget(m_lineEdit);
    layout->addWidget(m_button);

    QObject::connect(m_button, &QToolButton::clicked,
                     [this]() { showPopup(); });

}

void SearchableComboEditorTwo::destroyEditModeContext()
{
    if (m_popup) {
        m_popup->hide();
        m_popup->deleteLater();
        m_popup = nullptr;
    }

    // Удаляем явно — Qtitan удаляет m_editor (GridEditorBase),
    // но не m_container, у которого другой Qt-parent (GridEditorWidgetContainer)
    if (m_container) {
        delete m_container;   // дочерние m_lineEdit и m_button удалятся автоматически
        m_container = nullptr;
    }

    m_lineEdit = nullptr;
    m_button   = nullptr;
}

void SearchableComboEditorTwo::setValueToWidget(const QVariant& value)
{
    m_currentKey  = value.toInt();
    m_selectedKey = -1;
    m_clearChosen = false;
    updateDisplayText();
}

QVariant SearchableComboEditorTwo::getContextValue() const
{
    if (m_clearChosen)
        return QVariant(0);   // sentinel "не задано"

    if (m_selectedKey >= 0) {
        auto* repo = static_cast<SearchableComboRepositoryTwo*>(editorRepository());
        return repo->nameByKey(m_selectedKey);   // setData найдёт ключ по имени
    }

    return Qtitan::GridEditorBase::getContextValue();
}

bool SearchableComboEditorTwo::isContextModified()
{
    if (m_clearChosen || m_selectedKey >= 0)
        return true;
    return Qtitan::GridEditorBase::isContextModified();
}

void SearchableComboEditorTwo::updateDisplayText()
{
    if (!m_lineEdit) return;
    auto* repo = static_cast<SearchableComboRepositoryTwo*>(editorRepository());
    m_lineEdit->setText(repo ? repo->nameByKey(m_currentKey) : QString{});
}

void SearchableComboEditorTwo::showPopup()
{
    // m_popup либо nullptr (закрыт), либо жив (открыт повторно).
    // Если жив — просто поднимаем на передний план.
    if (m_popup) {
        m_popup->show();
        return;
    }

    auto* repo = static_cast<SearchableComboRepositoryTwo*>(editorRepository());
    if (!repo) return;

    QWidget* topLevel = repo->gridWidget()->window();
    m_popup = new SearchableComboPopupTwo(topLevel);
    m_popup->setItems(repo->items());
    m_popup->setCurrentKey(m_currentKey);

    // Позиционируем под контейнером
    const QPoint globalPos = m_container
                                 ? m_container->mapToGlobal(QPoint(0, m_container->height()))
                                 : QCursor::pos();

    const int minW = m_container
                         ? std::max(m_container->width(), 320)
                         : 320;

    m_popup->setMinimumWidth(minW);
    m_popup->move(globalPos);
    m_popup->show();
    m_popup->setFocus();

    QObject::connect(m_popup, &SearchableComboPopupTwo::itemSelected,
                     [this, repo](int key) {
                         m_selectedKey = key;
                         m_clearChosen = false;
                         if (m_lineEdit)
                             m_lineEdit->setText(repo->nameByKey(key));
                         setContextModified(true);
                         editorRepository()->view()->postEditor();
                     });

    QObject::connect(m_popup, &SearchableComboPopupTwo::clearSelected,
                     [this]() {
                         m_clearChosen = true;
                         m_selectedKey = -1;
                         if (m_lineEdit) m_lineEdit->clear();
                         setContextModified(true);
                         editorRepository()->view()->postEditor();
                     });

    QObject::connect(m_popup, &SearchableComboPopupTwo::editingCancelled,
                     [this]() {
                         m_selectedKey = -1;
                         m_clearChosen = false;
                         editorRepository()->view()->hideEditor();
                     });
}