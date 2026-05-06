#pragma once
#include "QtitanGrid.h"
#include "сolumnEditorInfo.h"

// SearchableComboRepository — репозиторий для nameref-колонок
class SearchableComboRepository : public Qtitan::GridEditorRepository
{
    Q_OBJECT
public:
    explicit SearchableComboRepository(
        const std::shared_ptr<ColumnEditorInfo::NameRefData>& nrd,
        QWidget* gridWidget)
        : Qtitan::GridEditorRepository()
        , m_nrd(nrd)
        , m_gridWidget(gridWidget)
    {}

    Qtitan::GridEditor* createEditor() override;

    /// Доступ к данным для popup
    const std::shared_ptr<ColumnEditorInfo::NameRefData>& nameRefData() const {
        return m_nrd;
    }

    /// Совместимость: gridWidget нужен в showPopup() для позиционирования
    QWidget* gridWidget() const { return m_gridWidget; }

    QString nameByKey(int key) const {
        return m_nrd->nameByKey(key);
    }

    void updateItems(const std::shared_ptr<ColumnEditorInfo::NameRefData>& nrd) {
        m_nrd = nrd;
    }

private:
    std::shared_ptr<ColumnEditorInfo::NameRefData> m_nrd;
    QWidget*                      m_gridWidget;
};