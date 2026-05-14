#include "autoFilterWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QAbstractItemModel>
#include <QtitanGrid.h>
#include "filterCell.h"

AutoFilterWidget::AutoFilterWidget(Qtitan::GridTableView* view, QWidget* parent)
    : QWidget(parent)
    , m_view(view)
{
    setFixedHeight(kRowHeight + 2);

    auto* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Заглушка под индикаторную колонку — ширина будет задана динамически
    m_indicatorSpacer = new QWidget(this);
    m_indicatorSpacer->setFixedWidth(1); // временно, уточнится в slot_syncLayout
    outerLayout->addWidget(m_indicatorSpacer);

    // Скроллируемая зона под колонки данных
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outerLayout->addWidget(m_scrollArea, 1);

    m_content = new QWidget;
    m_content->setMinimumHeight(kRowHeight);
    m_scrollArea->setWidget(m_content);
    m_scrollArea->setWidgetResizable(false);

    // Колонка переместилась
    connect(m_view, &Qtitan::GridViewBase::columnMoved,
            this, [this](const Qtitan::GridColumnBase*, int) {
                slot_syncLayout();
            });

    // Видимость колонки изменилась
    connect(m_view, &Qtitan::GridViewBase::columnVisibleChanged,
            this, [this](Qtitan::GridColumnBase*, bool) {
                rebuild();
            });

    // Горизонтальный скролл
    connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged,
            this, &AutoFilterWidget::slot_scrollChanged);

    // Вертикальный скроллбар появился/исчез — пересчитываем отступ
    connect(m_view->verticalScrollBar(), &QScrollBar::rangeChanged,
            this, [this](int, int) {
                slot_syncLayout();
            });

    rebuild();
}

void AutoFilterWidget::rebuild()
{
    qDeleteAll(m_cells);
    m_cells.clear();

    if (!m_view || !m_view->model())
        return;

    const int colCount = m_view->model()->columnCount();
    for (int i = 0; i < colCount; ++i) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(m_view->getColumn(i));
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

void AutoFilterWidget::clearAll()
{
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
    if (!isVisible() || !m_view)
        return;

    // Динамически вычисляем ширину индикаторной зоны
    const int iw = measureIndicatorWidth();
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
        auto* col = static_cast<Qtitan::GridTableColumn*>(m_view->getColumn(i));
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
        cell->resize(e.width, cell->height());
        xOffset += e.width;
    }

    m_content->setFixedSize(xOffset, kRowHeight);
}

int AutoFilterWidget::measureIndicatorWidth() const
{
    if (!m_view)
        return 0;

    // Суммарная ширина всех видимых колонок
    int colsTotal = 0;
    const int colCount = m_view->getColumnCount();
    for (int i = 0; i < colCount; ++i) {
        auto* col = m_view->getColumn(i);
        if (col && col->isVisible())
            colsTotal += col->width();
    }

    // Ширина области прокрутки горизонтального скроллбара:
    // maximum() - minimum() + pageStep() == полная ширина контента
    // pageStep() == ширина viewport
    auto* hsb = m_view->horizontalScrollBar();
    if (!hsb)
        return 0;

    // pageStep — это ширина видимой части (viewport без индикатора)
    const int viewportDataWidth = hsb->pageStep();
    if (viewportDataWidth <= 0)
        return 0;

    // Ширина виджета грида целиком
    QWidget* gridWidget = m_view->grid();
    if (!gridWidget)
        return 0;

    const int gridWidth = gridWidget->width();

    // Ширина вертикального скроллбара (если виден)
    auto* vsb = m_view->verticalScrollBar();
    const int vsbWidth = (vsb && vsb->isVisible()) ? vsb->width() : 0;

    // indicator = общая ширина грида − ширина данных-viewport − ширина vsb
    const int indicatorWidth = gridWidth - viewportDataWidth - vsbWidth;

    return qMax(0, indicatorWidth);
}