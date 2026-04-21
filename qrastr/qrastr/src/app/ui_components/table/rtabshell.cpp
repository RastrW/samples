#include "rtabshell.h"

#include "rgrid.h"
#include "rmodel.h"
#include "filtermanager.h"
#include "rtabcontroller.h"

#include <QVBoxLayout>
#include <QMenu>
#include <QCursor>
#include <QTimer>
#include <QGuiApplication>
#include <QClipboard>
#include <QModelIndex>
#include <algorithm>
#include "rGridTableView.h"
#include "filter/autoFilter/autoFilterWidget.h"

RtabShell::RtabShell(RGrid*                 grid,
                     Qtitan::GridTableView* view,
                     RModel*                model,
                     FilterManager*         filterManager,
                     RtabController*        controller,
                     bool                   withToolbar,
                     QWidget*               parent)
    : QWidget(parent)
    , m_grid(grid)
    , m_view(view)
    , m_model(model)
    , m_filterManager(filterManager)
{
    if (withToolbar) {
        buildToolbar(controller);
        //  Горячие клавиши
        RtabController::setupShortcuts(controller, m_grid);
    }

    buildLayout(withToolbar);

    // Синхронизация ширины строки автофильтра при изменении ширины колонок
    if (auto* viewH = qobject_cast<RGridTableView*>(m_view)) {
        connect(viewH, &RGridTableView::sig_columnWidthChanged,
                m_filterManager->widget(),
                [this](Qtitan::GridColumnBase*) {
                    m_filterManager->widget()->slot_syncLayout();
                });
    }

    // Сигнал от FilterManager::widget() → RtabController (маршрутизация)
    connect(m_filterManager->widget(), &AutoFilterWidget::sig_filterChanged,
            controller, &RtabController::slot_applyAutoFilter);

    // Статусбар: обновляем при каждом изменении строк
    connect(m_model, &QAbstractTableModel::rowsInserted,
            this, &RtabShell::slot_updateStatusLabel);
    connect(m_model, &QAbstractTableModel::rowsRemoved,
            this, &RtabShell::slot_updateStatusLabel);
    connect(m_model, &QAbstractTableModel::modelReset,
            this, &RtabShell::slot_updateStatusLabel);

    slot_updateStatusLabel();
}

void RtabShell::buildToolbar(RtabController* controller)
{
    m_toolbar = new QToolBar(this);
    m_toolbar->setIconSize(QSize(16, 16));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    // Qtitan::Grid имеет встроенную кнопку поиска/фильтрации
    // Операции с данными
    auto* actAddRow       = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"),   "");
    auto* actInsertRow    = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),   "");
    auto* actDeleteRow    = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),   "");
    auto* actDuplicateRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"),  "");
    auto* actGroupCorr    = m_toolbar->addAction(QIcon(":/images/column_edit.png"),                 "");
    auto* actAutoFilter   = m_toolbar->addAction(QIcon(":/images/new_style/filter.png"),            "");
    auto* actSearch       = m_toolbar->addAction(QIcon(":/images/new_style/search.png"),            "");
    auto* actCopy         = m_toolbar->addAction(QIcon(":/images/new_style/copy.png"),              "");

    actAddRow      ->setToolTip(tr("Добавить строку (Ctrl+A)"));
    actInsertRow   ->setToolTip(tr("Вставить строку (Ctrl+I)"));
    actDeleteRow   ->setToolTip(tr("Удалить строку (Ctrl+D)"));
    actDuplicateRow->setToolTip(tr("Дублировать строку (Ctrl+R)"));
    actGroupCorr   ->setToolTip(tr("Групповая корректировка"));
    actAutoFilter  ->setToolTip(tr("Показать/скрыть строку автофильтра"));
    actSearch      ->setToolTip(tr("Поиск по колонке"));
    actCopy        ->setToolTip(tr("Копировать таблицу в буфер обмена (Ctrl+C)"));
    actCopy        ->setShortcut(QKeySequence::Copy);

    actAutoFilter->setCheckable(true);
    actAutoFilter->setChecked(false);

    // Действия с данными — делегируются контроллеру
    connect(actAddRow,       &QAction::triggered, controller, &RtabController::slot_addRow);
    connect(actInsertRow,    &QAction::triggered, controller, &RtabController::slot_insertRow);
    connect(actDeleteRow,    &QAction::triggered, controller, &RtabController::slot_deleteRow);
    connect(actDuplicateRow, &QAction::triggered, controller, &RtabController::slot_duplicateRow);
    connect(actGroupCorr,    &QAction::triggered, controller, &RtabController::slot_groupCorrection);
    connect(actAutoFilter,   &QAction::toggled,   controller, &RtabController::slot_toggleAutoFilter);

    // UI-действия — остаются в шелле
    connect(actCopy,   &QAction::triggered, this, &RtabShell::slot_copyToClipboard);
    connect(actSearch, &QAction::triggered, this, &RtabShell::slot_showSearchMenu);
}

void RtabShell::buildLayout(bool withToolbar)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (withToolbar && m_toolbar)
        layout->addWidget(m_toolbar);

    layout->addWidget(m_filterManager->widget());
    layout->addWidget(m_grid);
    // ── Статусная строка под таблицей ──
    m_statusLabel = new QLabel(this);
    m_statusLabel->setContentsMargins(4, 2, 4, 2);
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(m_statusLabel);
}

void RtabShell::slot_updateStatusLabel()
{
    if (!m_statusLabel || !m_model) return;
    const int rows = m_model->rowCount();
    const int cols = m_view->getColumnCount(); // все столбцы dataset
    m_statusLabel->setText(tr("Строк: %1   Столбцов: %2").arg(rows).arg(cols));
}

void RtabShell::slot_showSearchMenu(){
    // Показать меню выбора колонки
    QMenu menu;
    for (int i = 0; i < m_view->getColumnCount(); ++i) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(m_view->getColumn(i));
        if (!col->isVisible()) continue;

        QAction* a = menu.addAction(col->caption());
        connect(a, &QAction::triggered, this, [this, col]() {
            m_view->options().setFindColumnList(
                {col->dataBinding()->columnName()});
            m_view->showFindPanel();
        });
    }
    menu.exec(QCursor::pos());
}

void RtabShell::slot_copyToClipboard()
{
    const int rowCount = m_model->rowCount();
    const int colCount = m_model->columnCount();
    if (rowCount == 0 || colCount == 0) return;

    // Собираем заголовки только для видимых колонок
    QStringList visibleHeaders;
    QVector<int> visibleCols;
    visibleCols.reserve(colCount);
    for (int c = 0; c < colCount; ++c) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(m_view->getColumn(c));
        if (col && col->isVisible()) {
            visibleCols.append(c);
            visibleHeaders.append(
                m_model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString());
        }
    }

    QString text;
    text.reserve(rowCount * visibleCols.size() * 8); // приблизительно
    text += visibleHeaders.join('\t') + '\n';
    // ── Определяем диапазон: выделение или вся таблица ──────────────────────
    QModelIndexList selected = m_view->selection()->selectedIndexes();
    if (!selected.isEmpty()) {
        // Структура для сортировки: {row, visColPos, value}
        struct Cell { int row; int visCol; QString value; };
        std::vector<Cell> cells;
        cells.reserve(selected.size());

        for (const QModelIndex& idx : selected) {
            const int visCol = visibleCols.indexOf(idx.column());
            if (visCol < 0) continue;
            cells.push_back({ idx.row(), visCol,
                             m_model->data(idx, Qt::DisplayRole).toString() });
        }

        // Сортируем по (row, visCol) — std::vector + std::sort
        std::sort(cells.begin(), cells.end(), [](const Cell& a, const Cell& b) {
            return a.row != b.row ? a.row < b.row : a.visCol < b.visCol;
        });

        int prevRow = -1;
        QStringList rowData;
        rowData.reserve(visibleCols.size());

        auto flushRow = [&]() {
            if (prevRow >= 0) text += rowData.join('\t') + '\n';
        };

        for (const Cell& c : cells) {
            if (c.row != prevRow) {
                flushRow();
                rowData.assign(visibleCols.size(), QString());
                prevRow = c.row;
            }
            rowData[c.visCol] = c.value;
        }
        flushRow();
    } else {
        // Копируем все строки
        for (int r = 0; r < rowCount; ++r) {
            QStringList rowData;
            rowData.reserve(visibleCols.size());
            for (int c : visibleCols)
                rowData.append(m_model->data(m_model->index(r, c), Qt::DisplayRole).toString());
            text += rowData.join('\t') + '\n';
        }
    }

    QGuiApplication::clipboard()->setText(text);
    // Краткая индикация в статусной строке
    if (m_statusLabel) {
        const QString prev = m_statusLabel->text();
        m_statusLabel->setText(tr("✓ Скопировано в буфер"));
        QTimer::singleShot(2000, this, [this, prev]() {
            if (m_statusLabel) m_statusLabel->setText(prev);
        });
    }
}