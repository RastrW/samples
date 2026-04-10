#include "autoFilterWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QAbstractItemModel>
#include <QtitanGrid.h>
#include "filterCell.h"

AutoFilterWidget::AutoFilterWidget(GridTableView* view, QWidget* parent)
    : QWidget(parent)
    , m_view(view)
{
    setFixedHeight(kRowHeight + 2);

    auto* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Фиксированная заглушка под индикаторную колонку грида
    m_indicatorSpacer = new QWidget(this);
    m_indicatorSpacer->setFixedWidth(kBorderX);
    outerLayout->addWidget(m_indicatorSpacer);

    // Скроллируемая зона под данные колонки
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outerLayout->addWidget(m_scrollArea, 1);

    m_content = new QWidget;
    m_content->setMinimumHeight(kRowHeight);
    m_scrollArea->setWidget(m_content);
    m_scrollArea->setWidgetResizable(false);

    // ===== СИГНАЛЫ QTITAN =====

    // Основной сигнал: структура колонок изменилась
    // Сюда входит изменение ширины, видимости и т.д.
    connect(m_view, &GridViewBase::columnsUpdated,
            this, &AutoFilterWidget::slot_syncLayout);

    // Дополнительный сигнал: колонна переместилась
    // (может влиять на layout)
    connect(m_view, &GridViewBase::columnMoved,
            this, [this](const GridColumnBase*, int) {
                slot_syncLayout();
            });

    // Когда видимость колонки меняется — пересоздаём ячейки
    connect(m_view, &GridViewBase::columnVisibleChanged,
            this, [this](GridColumnBase*, bool) {
                rebuild();;
            });

    // Горизонтальный скролл
    connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged,
            this, &AutoFilterWidget::slot_scrollChanged);

    rebuild();
}

void AutoFilterWidget::rebuild()
{
    m_indicatorMeasured = false;

    qDeleteAll(m_cells);
    m_cells.clear();

    if (!m_view || !m_view->model())
        return;

    const int colCount = m_view->model()->columnCount();
    for (int i = 0; i < colCount; ++i) {
        auto* col = static_cast<GridTableColumn*>(m_view->getColumn(i));
        if (!col || !col->isVisible())
            continue;

        const bool isNumeric = col->property("isNumeric").toBool();
        const bool isBool    = col->property("isBool").toBool();

        auto* cell = new FilterCell(isNumeric, isBool, m_content);
        cell->setPlaceholderText(col->caption());
        cell->setFixedHeight(kRowHeight);

        const int capturedIdx = i;
        connect(cell, &FilterCell::sig_filterChanged, this,
                [this, capturedIdx](const FilterRule& rule) {
                    emit sig_filterChanged(capturedIdx, rule);
                });

        m_cells[i] = cell;
        cell->show();
    }

    slot_syncLayout();
}

void AutoFilterWidget::clearAll(){
    for (FilterCell* cell : m_cells) {
        QSignalBlocker blk(cell);
        cell->clear();
    }
    for (auto it = m_cells.constBegin(); it != m_cells.constEnd(); ++it)
        emit sig_filterChanged(it.key(), FilterRule{});
}

void AutoFilterWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Когда виджет показывается (после скрытия), синхронизируем
    // гарантированно. Это важно при переоткрытии фильтра.
    QTimer::singleShot(0, this, &AutoFilterWidget::slot_syncLayout);
}

void AutoFilterWidget::slot_scrollChanged(int value){
    m_scrollArea->horizontalScrollBar()->setValue(value);
}

void AutoFilterWidget::slot_syncLayout()
{
    if (!m_view)
        return;

    const int iw = kBorderX;
    if (iw > 0 && m_indicatorSpacer->width() != iw)
        m_indicatorSpacer->setFixedWidth(iw);

    const int colCount = m_view->model() ? m_view->model()->columnCount() : 0;

    struct ColEntry {
        int modelIdx;
        int visualIdx;
        int width;
    };

    QVector<ColEntry> visible;
    visible.reserve(colCount);

    for (int i = 0; i < colCount; ++i) {
        auto* col = static_cast<GridTableColumn*>(m_view->getColumn(i));
        if (!col || !col->isVisible())
            continue;

        visible.push_back({ i, col->visualIndex(), col->width() });
    }

    std::sort(visible.begin(), visible.end(),
              [](const ColEntry& a, const ColEntry& b) {
                  return a.visualIdx < b.visualIdx;
              });

    int xOffset = 0;
    for (const ColEntry& e : visible) {
        if (!m_cells.contains(e.modelIdx))
            continue;

        FilterCell* cell = m_cells[e.modelIdx];
        cell->move(xOffset, 1);
        cell->setFixedWidth(e.width);
        // Гарантируем вызов resizeEvent в FilterCell
        cell->resize(e.width, cell->height());

        xOffset += e.width;
    }

    m_content->setFixedSize(xOffset > 0 ? xOffset : 1, kRowHeight);
}