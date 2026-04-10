#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QTimer>
#include <QMap>
#include "filterRule.h"

namespace Qtitan {class GridTableView;}
class FilterCell;

/// @class Строка автофильтра над заголовком таблицы QTitan.
/// Виджет создаёт по одному QLineEdit на каждую видимую колонку грида.
/// Позиционирование — абсолютное (через QWidget::move/resize), синхронизируется
/// с шириной колонок по сигналам QTitan и по горизонтальному скроллу грида.
class AutoFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutoFilterWidget(Qtitan::GridTableView* view,
                              QWidget* parent = nullptr);
    /// @brief Пересоздать ячейки.
    void rebuild();
    /// @brief Очистить все введённые условия.
    void clearAll();
signals:
    /// Испускается при изменении любого поля ввода.
    /// @param colIndex — модельный индекс колонки
    /// @param text     — текущий текст поля
    void sig_filterChanged(int colIndex, const FilterRule& rule);

public slots:
    void slot_scrollChanged(int value);

protected:
    // Отслеживаем показ виджета
    void showEvent(QShowEvent* event) override;

private slots:
    void slot_syncLayout();

private:
    Qtitan::GridTableView* m_view = nullptr;
    QScrollArea*           m_scrollArea = nullptr;
    QWidget*               m_content = nullptr;
    bool                   m_indicatorMeasured = false;
    QWidget*               m_indicatorSpacer = nullptr;

    QMap<int, FilterCell*> m_cells; // modelIndex → cell
};