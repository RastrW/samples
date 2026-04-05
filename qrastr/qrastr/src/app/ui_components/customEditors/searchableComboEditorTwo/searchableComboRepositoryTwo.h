#pragma once
#include "QtitanGrid.h"

// SearchableComboRepositoryTwo — репозиторий для nameref-колонок
class SearchableComboRepositoryTwo : public Qtitan::GridEditorRepository
{
    Q_OBJECT
public:
    explicit SearchableComboRepositoryTwo(
        const std::map<size_t, std::string>& items,
        QWidget* gridWidget)
        : Qtitan::GridEditorRepository()
        , m_items(items)
        , m_gridWidget(gridWidget)
    {}

    Qtitan::GridEditor* createEditor() override;

    const std::map<size_t, std::string>& items()      const { return m_items;      }
    QWidget*                             gridWidget()  const { return m_gridWidget; }

    /// Поиск имени по ключу (используется для отображения текущего значения)
    QString nameByKey(int key) const {
        auto it = m_items.find(static_cast<size_t>(key));
        return it != m_items.end()
                   ? QString::fromStdString(it->second)
                   : QString{};
    }

private:
    std::map<size_t, std::string> m_items;
    QWidget*                      m_gridWidget;
};