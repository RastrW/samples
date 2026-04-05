#pragma once
#include "QtitanGrid.h"

/// Репозиторий: хранит список элементов, фабрика редактора
class SearchableComboRepository : public Qtitan::GridEditorRepository
{
    Q_OBJECT
public:
    explicit SearchableComboRepository(const QStringList& items, QWidget* gridWidget)
        : Qtitan::GridEditorRepository()
        , m_items(items)
        , m_gridWidget(gridWidget)
    {}

    Qtitan::GridEditor* createEditor() override;

    const QStringList& items() const { return m_items; }
    QWidget* gridWidget() const { return m_gridWidget; }

private:
    QStringList m_items;
    QWidget*    m_gridWidget;
};