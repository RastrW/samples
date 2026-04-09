#include "autoFilterWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QAbstractItemModel>
#include <QtitanGrid.h>

static constexpr int kRowHeight = 25;
// смещение от левого края виджета до первой колонки
static constexpr int kBorderX   = 57;

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
    outerLayout->addWidget(m_scrollArea, 1); // stretch=1, занимает остаток

    m_content = new QWidget;
    m_content->setMinimumHeight(kRowHeight);
    m_scrollArea->setWidget(m_content);
    m_scrollArea->setWidgetResizable(false);

    // Сигналы QTitan
    connect(m_view, &GridViewBase::columnsUpdated,
            this, &AutoFilterWidget::slot_syncLayout);
    connect(m_view, &GridViewBase::columnVisibleChanged,
            this, [this](GridColumnBase*, bool) { rebuild(); });

    // Ресайз колонок → меняется scroll range
    connect(m_view->horizontalScrollBar(), &QScrollBar::rangeChanged,
            this, &AutoFilterWidget::slot_syncLayout);

    // Следим за resize грида — он происходит при открытии таблицы,
    // изменении размера окна и при первом показе
    m_view->grid()->installEventFilter(this);

    rebuild();
}

void AutoFilterWidget::rebuild()
{
    m_indicatorMeasured = false; // структура колонок изменилась

    // Удаляем старые редакторы
    for (QLineEdit* ed : m_editors) {
        ed->hide();
        ed->deleteLater();
    }
    m_editors.clear();

    if (!m_view || !m_view->model()) return;

    const int colCount = m_view->model()->columnCount();
    for (int i = 0; i < colCount; ++i) {
        GridTableColumn* col = static_cast<GridTableColumn*>(m_view->getColumn(i));
        if (!col || !col->isVisible()) continue;

        auto* ed = new QLineEdit(m_content);
        ed->setPlaceholderText(col->caption());
        ed->setFixedHeight(kRowHeight);
        ed->setFrame(true);
        ed->setContentsMargins(2, 0, 2, 0);

        // Стиль — слегка выделить как строку фильтра
        ed->setStyleSheet(
            "QLineEdit {"
            "  background: palette(base);"
            "  border: 1px solid palette(mid);"
            "  border-radius: 2px;"
            "  padding: 0 3px;"
            "  font-style: italic;"
            "  color: palette(text);"
            "}"
            "QLineEdit:focus {"
            "  border: 1px solid palette(highlight);"
            "  font-style: normal;"
            "}"
        );

        const int capturedIdx = i;
        connect(ed, &QLineEdit::textChanged, this,
                [this, capturedIdx](const QString& t) {
            emit sig_filterChanged(capturedIdx, t);
        });

        m_editors[i] = ed;
        ed->show();
    }

    slot_syncLayout();
}

void AutoFilterWidget::clearAll(){

    for (QLineEdit* ed : m_editors) {
        QSignalBlocker blk(ed);
        ed->clear();
    }
    // Испускаем filterChanged с пустой строкой для всех колонок,
    // чтобы контроллер знал о сбросе
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it)
        emit sig_filterChanged(it.key(), QString());
}

void AutoFilterWidget::slot_scrollChanged(int value){

    m_scrollArea->horizontalScrollBar()->setValue(value);
}

void AutoFilterWidget::slot_syncLayout()
{
    if (!m_view) return;

    const int iw = kBorderX;
    if (iw > 0 && m_indicatorSpacer->width() != iw)
        m_indicatorSpacer->setFixedWidth(iw);

    const int colCount = m_view->model() ? m_view->model()->columnCount() : 0;

    struct ColEntry { int modelIdx; int visualIdx; int width; };
    QVector<ColEntry> visible;
    for (int i = 0; i < colCount; ++i) {
        auto* col = static_cast<GridTableColumn*>(m_view->getColumn(i));
        if (!col || !col->isVisible()) continue;
        visible.push_back({ i, col->visualIndex(), col->width() });
    }
    std::sort(visible.begin(), visible.end(),
              [](const ColEntry& a, const ColEntry& b) {
                  return a.visualIdx < b.visualIdx;
              });

    int xOffset = 0;
    for (const ColEntry& e : visible) {
        if (!m_editors.contains(e.modelIdx)) continue;
        QLineEdit* ed = m_editors[e.modelIdx];
        ed->move(xOffset, 1);
        ed->setFixedWidth(e.width);
        xOffset += e.width;
    }
    m_content->setFixedSize(xOffset ? xOffset : 1, kRowHeight);
}

void AutoFilterWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Грид уже выложен к моменту show() AutoFilterWidget,
    // поэтому измеряем индикатор здесь
    //m_indicatorMeasured = false; // сбросить чтобы перемерить
    //slot_syncLayout();
}

bool AutoFilterWidget::eventFilter(QObject* obj, QEvent* event){
    return QWidget::eventFilter(obj, event);
}