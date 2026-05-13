#pragma once
#include <QObject>
#include "table/tableIndexTypes.h"

class RModel;
class AutoFilterWidget;
class AutoFilterCondition;
struct FilterRule;
namespace Qtitan { class GridTableView; class GridFilterCondition;}

/// @brief Управляет выборкой и автофильтром, пересобирает общий фильтр грида.
class FilterManager : public QObject
{
    Q_OBJECT
public:
    explicit FilterManager(Qtitan::GridTableView* view, RModel* model,
                           QWidget* parentWidget);

    /// Виджет строки фильтра — добавляется во внешний layout.
    AutoFilterWidget* widget() const { return m_autoFilter.get(); }

    /// Вызывается из slot_toggleAutoFilter.
    void toggle(bool checked);
    /// Вызывается из slot_applyAutoFilter.
    void applyRule(int colIndex, const FilterRule& rule);
    /// Применяет строку выборки (результат SelectionDialog).
    void setSelection(const std::string& selection);
    /// Полный сброс после EndResetModel.
    void resetAfterModelReset();

    const std::string& currentSelection() const { return m_selection; }
public slots:
    void slot_openSelection(ModelIndex col);
private slots:
    void slot_setFiltrForSelection(std::string selection);
    // Вызывается когда QTitan записал новое условие в историю.
    // На этот момент filter()->condition() уже содержит новое встроенное условие.
    void slot_onBuiltinFilterChanged();
private:
    /// @brief Пересобирает общий фильтр (AND выборки + AND автофильтра) и
    ///        передаёт его в m_view->filter().
    void rebuildCombinedFilter();

    Qtitan::GridTableView* m_view;
    RModel*                m_model;
    QWidget*               m_parentWidget;
    std::unique_ptr<AutoFilterWidget> m_autoFilter;
    std::unique_ptr<AutoFilterCondition> m_autoFilterCond;
    // Клон последнего известного встроенного условия QTitan.
    // Владеем им сами (delete при замене).
    Qtitan::GridFilterCondition*      m_builtinCondition {nullptr};

    // Защита от рекурсии: наш setCondition тоже триггерит history::changed
    bool m_rebuildingFilter {false};

    std::string m_selection;        // Текущая выборка
};