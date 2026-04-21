#pragma once
#include <QObject>

class RModel;
class AutoFilterWidget;
class AutoFilterCondition;
struct FilterRule;
namespace Qtitan { class GridTableView; }

/// @brief Управляет выборкой и автофильтром, пересобирает общий фильтр грида.
class FilterManager : public QObject
{
    Q_OBJECT
public:
    explicit FilterManager(Qtitan::GridTableView* view, RModel* model,
                           QWidget* parentWidget);

    /// Виджет строки фильтра — добавляется во внешний layout.
    AutoFilterWidget* widget() const { return m_autoFilter; }

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
    void slot_openSelection(int col);
private slots:
    void slot_setFiltrForSelection(std::string selection);
private:
    /// @brief Пересобирает общий фильтр (AND выборки + AND автофильтра) и
    ///        передаёт его в m_view->filter().
    void rebuildCombinedFilter();

    Qtitan::GridTableView* m_view;
    RModel*                m_model;
    QWidget*               m_parentWidget;
    AutoFilterWidget*    m_autoFilter    {nullptr};
    AutoFilterCondition* m_autoFilterCond{nullptr};

    QString     m_selectionFilter;  // последняя строка выборки
    std::string m_selection;        // Текущая выборка
};