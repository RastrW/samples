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

    // ── QScrollArea без полос прокрутки — прокруткой управляет грид ──
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_content = new QWidget;
    m_content->setMinimumHeight(kRowHeight);
    m_scrollArea->setWidget(m_content);
    m_scrollArea->setWidgetResizable(false);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_scrollArea);

    // ── Сигналы QTitan ──
    // любые изменения колонок (ширина, порядок, и т.д.)
    connect(m_view, &GridViewBase::columnsUpdated,
            this, &AutoFilterWidget::slot_syncLayout);
    connect(m_view, &GridViewBase::columnVisibleChanged,
            this, [this](GridColumnBase*, bool) {
                rebuild();
            });

    rebuild();
}

void AutoFilterWidget::rebuild()
{
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

    // Считаем суммарную ширину
    int totalWidth = kBorderX;
    const int colCount = m_view->model() ? m_view->model()->columnCount() : 0;

    // Собираем видимые колонки в порядке visualIndex
    struct ColEntry { int modelIdx; int visualIdx; int width; };
    QVector<ColEntry> visible;
    for (int i = 0; i < colCount; ++i) {
        auto* col = static_cast<GridTableColumn*>(m_view->getColumn(i));
        if (!col || !col->isVisible()) continue;
        visible.push_back({ i, col->visualIndex(), col->width() });
    }
    std::sort(visible.begin(), visible.end(),
              [](const ColEntry& a, const ColEntry& b){ return a.visualIdx < b.visualIdx; });

    // Учитываем ширину индикаторной колонки (row indicator) QTitan
    int xOffset = kBorderX;

    for (const ColEntry& e : visible) {
        if (!m_editors.contains(e.modelIdx)) continue;
        QLineEdit* ed = m_editors[e.modelIdx];
        ed->move(xOffset, 1);
        ed->setFixedWidth(e.width);
        xOffset += e.width;
    }

    totalWidth = xOffset;
    m_content->setFixedSize(totalWidth, kRowHeight);
}