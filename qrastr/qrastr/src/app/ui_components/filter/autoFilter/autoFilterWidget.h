#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QTimer>
#include <QMap>

namespace Qtitan {class GridTableView;}

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

    /// @brief Пересоздать ячейки (вызывать после endResetModel или изменения
    ///        набора колонок).
    void rebuild();
    /// @brief Очистить все введённые условия (текст в полях).
    void clearAll();
signals:
    /// Испускается при изменении любого поля ввода.
    /// @param colIndex — модельный индекс колонки
    /// @param text     — текущий текст поля
    void sig_filterChanged(int colIndex, const QString& text);

public slots:
    void slot_scrollChanged(int value);
private slots:
    void slot_syncLayout();
private:
    Qtitan::GridTableView* m_view;
    QScrollArea*           m_scrollArea;
    QWidget*               m_content;

    // modelIndex → QLineEdit
    QMap<int, QLineEdit*>  m_editors;
};