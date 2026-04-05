#include "searchableComboEditorTwo.h"
#include "searchableComboRepositoryTwo.h"
#include "searchableComboPopupTwo.h"
#include <QVBoxLayout>
#include <QApplication>

void SearchableComboEditorTwo::createEditModeContext()
{
    m_selectedKey = -1;
    m_clearChosen = false;
    m_currentKey  = getContextValue().toInt();
    showPopup();
}

void SearchableComboEditorTwo::destroyEditModeContext()
{
    editorRepository()->setImmediatePost(true);
    if (m_popup) {
        m_popup->hide();
        m_popup->deleteLater();
        m_popup = nullptr;
    }
}

QVariant SearchableComboEditorTwo::getContextValue() const
{
    if (m_clearChosen)
        // "Не задано" — возвращаем пустую строку;
        // setData не найдёт совпадений в namerefMap → запишет 0.
        // При необходимости поменяйте на нужный sentinel-ключ.
        return QString{};

    if (m_selectedKey >= 0) {
        // Возвращаем строку-имя, чтобы setData корректно нашёл ключ
        const auto* repo = static_cast<SearchableComboRepositoryTwo*>(editorRepository());
        return repo->nameByKey(m_selectedKey);
    }

    return Qtitan::GridEditorBase::getContextValue();
}

bool SearchableComboEditorTwo::isContextModified()
{
    if (m_clearChosen || m_selectedKey >= 0)
        return true;
    return Qtitan::GridEditorBase::isContextModified();
}

void SearchableComboEditorTwo::setValueToWidget(const QVariant& value)
{
    m_currentKey = value.toInt();
    if (m_popup)
        m_popup->setCurrentKey(m_currentKey);
}

void SearchableComboEditorTwo::showPopup()
{
    auto* repo = static_cast<SearchableComboRepositoryTwo*>(editorRepository());
    if (!repo) return;

    editorRepository()->setImmediatePost(false);

    m_popup = new SearchableComboPopupTwo(repo->gridWidget()->window());
    m_popup->setItems(repo->items());
    m_popup->setCurrentKey(m_currentKey);

    // ── Позиционирование: левый верхний угол popup = левый нижний угол ячейки
    // site() в QTitan — это QWidget, занимающий ровно прямоугольник ячейки.
    QPoint globalBottomLeft;
    if (auto* siteWidget = dynamic_cast<QWidget*>(site())) {
        globalBottomLeft = siteWidget->mapToGlobal(
            QPoint(0, siteWidget->height()));
    } else {
        globalBottomLeft = QCursor::pos();  // fallback
    }

    m_popup->setMinimumWidth(qMax(site()->geometry().width(), 320));
    m_popup->move(globalBottomLeft);
    m_popup->show();
    m_popup->setFocus();

    // ── Сигналы ──────────────────────────────────────────────────────────────
    QObject::connect(m_popup, &SearchableComboPopupTwo::itemSelected,
                     [this](int key) {
                         m_selectedKey = key;
                         m_clearChosen = false;
                         setContextModified(true);
                         editorRepository()->view()->postEditor();
                     });

    QObject::connect(m_popup, &SearchableComboPopupTwo::clearSelected,
                     [this]() {
                         m_clearChosen = true;
                         m_selectedKey = -1;
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