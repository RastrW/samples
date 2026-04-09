#include "rtabwidget.h"
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QLineEdit>
#include <QScrollBar>
//#include "filtertableheader.h"
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QDateTime>
#include <QProgressDialog>
#include "qastra.h"
#include <QShortcut>
#include <QPalette>
#include <QStandardItem>
#include "qastra.h"
#include <QShortcut>
#include <QPalette>
#include <QLabel>
#include "linkedform.h"

#include <QtitanGrid.h>
#include <utils.h>
#include <QAbstractItemModelTester>
#include <DockManager.h>
#include <QCloseEvent>
#include <QTableView>
#include <QMessageBox>

#include "tables/selectionDialog.h"
#include "tables/groupCorrectionDialog.h"
#include "tables/exportCSVdialog.h"
#include "tables/importCSV2dialog.h"
#include <QToolBar>
#include "linkedformcontroller.h"
#include "tables/colPropDialog.h"
#include "contextMenuBuilder.h"
#include "filter/customFilterCondition.h"
#include "condFormatController.h"
#include "rmodel.h"
#include "rtablesdatamanager.h"
#include "qastra.h"
#include "rdata.h"
#include "QDataBlocks.h"
#include "customEditors/searchableComboEditorTwo/searchableComboRepositoryTwo.h"
#include "filter/autoFilter/autoFilterWidget.h"
#include "filter/autoFilter/autoFilterCondition.h"
#include "filter/autoFilter/filterRuleParser.h"

void dumpShortcuts(QWidget* root, const QString& tag)
{
    qDebug() << "=== Dump shortcuts:" << tag << "===";

    auto dump = [](QWidget* w) {
        for (QAction* act : w->actions()) {
            if (!act->shortcut().isEmpty()) {
                qDebug() << "Widget:" << w
                         << "Action:" << act->text()
                         << "Shortcut:" << act->shortcut().toString();
            }
        }
    };

    dump(root);

    for (QObject* child : root->children()) {
        if (auto* w = qobject_cast<QWidget*>(child))
            dump(w);
    }
}

RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm, RTablesDataManager* pRTDM,
                       ads::CDockManager* pDockManager, QWidget *parent)
    : QWidget(parent),
    m_UIForm {UIForm},
    m_pqastra {pqastra},
    m_pRTDM {pRTDM},
    m_DockManager {pDockManager}
{
    //  Настройка QTitan Grid
    Grid::loadTranslation();
    m_grid = new Qtitan::Grid(this);
    if (m_UIForm.Vertical()){
        m_grid->setViewType(Qtitan::Grid::BandedTableViewVertical);
    }else{
        m_grid->setViewType(Qtitan::Grid::TableView);
    }
    m_view = m_grid->view< Qtitan::GridTableView>();

    if (m_UIForm.Vertical()){
        m_view->tableOptions().setRowSizingEnabled(true);
    }else{
        m_view->tableOptions().setRowFrozenButtonVisible(true);
        m_view->tableOptions().setFrozenPlaceQuickSelection(true);
        m_view->tableOptions().setRowsQuickSelection(true);
    }

    m_view->options().setGridLines(Qtitan::LinesBoth);
    m_view->options().setGridLineWidth(1);
    //user can select several cells at time. Hold shift key to select multiple cells.
    m_view->options().setColumnHidingOnGroupingEnabled(false);
    // Выделение: MultiRowSelection + rubber-band через indicator-колонку
    m_view->options().setSelectionPolicy(Qtitan::GridViewOptions::MultiCellSelection);
    m_view->options().setRubberBandSelection(true);
    // Drag отключаем — он конкурирует с rubber-band
    m_view->options().setDragEnabled(false);
    // Sets the value that indicates whether the filter panel can automatically hide or not.
    m_view->options().setFilterAutoHide(true);
    // Sets the painting the doted frame around the cell with focus to the enabled. By default frame is enabled.
    //m_view->options().setFocusFrameEnabled(true);
    // Sets the visibility status of the grid grouping panel to groupsHeader.
    m_view->options().setGroupsHeader(false);
    // ScrollByPixel значительно быстрее ScrollByItem при большом числе строк:
    // не требует пересчёта высот всех строк при каждом шаге скроллинга.
    m_view->options().setScrollRowStyle(Qtitan::ScrollItemStyle::ScrollByPixel);
    // Enables or disables wait cursor if grid is busy for lengthy operations with data like sorting or grouping.
    m_view->options().setShowWaitCursor(true);
    m_view->tableOptions().setColumnsHeader(true);

    ///@note создание модели обязатель до меню!
    createModel();

    //  Горячие клавиши
    setupShortcuts();

    m_menuBuilder = std::make_unique<ContextMenuBuilder>(
        m_view,
        m_linkedFormCtrl.get(),
        this);
    m_menuBuilder->initMenu(this);

    setupConnections();

    //dumpShortcuts(m_grid, "before clear");
    // Снимаем F5/Delete со встроенных action-ов QTitan
    auto& acts = m_view->actions();

    // удалить shortcut у DeleteRowAction
    if (acts.contains(Qtitan::GridViewBase::DeleteRowAction)) {
        acts[Qtitan::GridViewBase::DeleteRowAction]->setShortcut(QKeySequence());
    }

    // если F5 где-то используется (например Find/Refresh)
    for (auto it = acts.begin(); it != acts.end(); ++it) {
        if (it.value()->shortcut() == QKeySequence(Qt::Key_F5)) {
            it.value()->setShortcut(QKeySequence());
        }
    }

    //dumpShortcuts(m_grid, "after clear");
}

RtabWidget::~RtabWidget() {}

QWidget* RtabWidget::createDockContent(bool addToolbar) {
    QWidget* wrapper = new QWidget(this);
    auto* layout = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (addToolbar) {
        setupToolbar();
        layout->addWidget(m_toolbar);
    }
    // ── Строка автофильтра (скрыта по умолчанию) ──
    m_autoFilterCond = new AutoFilterCondition(m_view->filter());
    m_autoFilter = new AutoFilterWidget(m_view, wrapper);
    m_autoFilter->setVisible(false);
    layout->addWidget(m_autoFilter);

    // Синхронизация горизонтальной прокрутки
    // QTitan не даёт прямого доступа к QScrollBar, поэтому ищем через children.
    // Если QTitan обновится и сломается — достаточно поправить этот блок.
    if (auto* hBar = m_grid->findChild<QScrollBar*>(QString(), Qt::FindDirectChildrenOnly)) {
        connect(hBar, &QScrollBar::valueChanged,
                m_autoFilter, &AutoFilterWidget::slot_scrollChanged);
    } else {
        // Fallback: ищем рекурсивно по ориентации
        const auto bars = m_grid->findChildren<QScrollBar*>();
        for (QScrollBar* bar : bars) {
            if (bar->orientation() == Qt::Horizontal) {
                connect(bar, &QScrollBar::valueChanged,
                        m_autoFilter, &AutoFilterWidget::slot_scrollChanged);
                break;
            }
        }
    }

    connect(m_autoFilter, &AutoFilterWidget::sig_filterChanged,
            this, &RtabWidget::slot_applyAutoFilter);

    layout->addWidget(m_grid);

    // ── Статусная строка под таблицей ──
    m_statusLabel = new QLabel(wrapper);
    m_statusLabel->setContentsMargins(4, 2, 4, 2);
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(m_statusLabel);

    slot_updateStatusLabel();   // начальное значение

    // Обновляем счётчик при любом изменении строк
    connect(m_model.get(), &QAbstractTableModel::rowsInserted,
            this, &RtabWidget::slot_updateStatusLabel);
    connect(m_model.get(), &QAbstractTableModel::rowsRemoved,
            this, &RtabWidget::slot_updateStatusLabel);
    connect(m_model.get(), &QAbstractTableModel::modelReset,
            this, &RtabWidget::slot_updateStatusLabel);

    return wrapper;
}

void RtabWidget::slot_updateStatusLabel() {
    if (!m_statusLabel || !m_model) return;
    const int rows = m_model->rowCount();
    const int cols = m_view->getColumnCount();   // все столбцы dataset
    m_statusLabel->setText(tr("Строк: %1   Столбцов: %2").arg(rows).arg(cols));
}

void RtabWidget::setupConnections(){
    //RTablesDataManager -> RModel
    connect(m_pRTDM, &RTablesDataManager::sig_dataChanged,this->m_model.get(),
            &RModel::slot_DataChanged);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this->m_model.get(),
            &RModel::slot_BeginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this->m_model.get(),
            &RModel::slot_EndResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginInsertRow,this->m_model.get(),
            &RModel::slot_BeginInsertRow);
    connect(m_pRTDM, &RTablesDataManager::sig_EndInsertRow,this->m_model.get(),
            &RModel::slot_EndInsertRow);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginRemoveRows,this->m_model.get(),
            &RModel::slot_BeginRemoveRows);
    connect(m_pRTDM, &RTablesDataManager::sig_EndRemoveRows,this->m_model.get(),
            &RModel::slot_EndRemoveRows);
    //RTablesDataManager -> RtabWidget
    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this,
            &RtabWidget::slot_beginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this,
            &RtabWidget::slot_endResetModel);
    //ContextMenuBuilder -> RtabWidget
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_addRow,
            this, &RtabWidget::slot_addRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_insertRow,
            this, &RtabWidget::slot_insertRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_deleteRow,
            this, &RtabWidget::slot_deleteRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_duplicateRow,
            this, &RtabWidget::slot_duplicateRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_groupCorrection,
            this, &RtabWidget::slot_groupCorrection);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_colProp,
            this, &RtabWidget::slot_openColProp);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_selection,
            this, &RtabWidget::slot_openSelection);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_exportCsv,
            this, &RtabWidget::slot_openExportCSVForm);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_importCsv,
            this, &RtabWidget::slot_openImportCSVForm);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_widthByTemplate,
            this, &RtabWidget::slot_widthByTemplate);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_widthByData,
            this, &RtabWidget::slot_widthByData);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_directCodeToggle,
            this, &RtabWidget::slot_directCodeToggle);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_condFormatsEdit,
            this, &RtabWidget::slot_condFormatsEdit);

    //QTitan
    //Connect Grid's context menu handler.
    connect(m_view, &GridTableView::contextMenu, this, &RtabWidget::slot_contextMenu);
}

void RtabWidget::setPyHlp(std::shared_ptr<PyHlp> pPyHlp){
    pPyHlp_ = pPyHlp;
    if (m_linkedFormCtrl){
        m_linkedFormCtrl->setPyHlp(pPyHlp);
    }
}

int RtabWidget::getLongValue(const std::string& key, long row){
    int col = m_model->getRdata()->mCols_.at(key);
    return std::visit(ToLong(), m_model->getRdata()->pnparray_->Get(row,col));
}

void RtabWidget::applyLinkedFormFromController(const LinkedForm& lf)
{
    // Точка входа для родительского LinkedFormController:
    // он вызывает этот метод на дочернем RtabWidget.
    m_linkedFormCtrl->applyLinkedForm(lf);
}

void RtabWidget::setupToolbar() {
    m_toolbar = new QToolBar(this);
    m_toolbar->setIconSize(QSize(16, 16));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Qtitan::Grid имеет встроенную кнопку поиска/фильтрации
    // Операции с данными
    m_actAddRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"), "");
    m_actInsertRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"), "");
    m_actDeleteRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"), "");
    m_actDuplicateRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"), "");
    m_groupCorrection = m_toolbar->addAction(QIcon(":/images/column_edit.png"), "");
    m_actAutoFilter = m_toolbar->addAction(QIcon(":/images/new_style/filter.png"),"");

    m_actAddRow->setToolTip(tr("Добавить строку (Ctrl+A)"));
    m_actInsertRow->setToolTip(tr("Вставить строку (Ctrl+I)"));
    m_actDeleteRow->setToolTip(tr("Удалить строку (Ctrl+D)"));
    m_actDuplicateRow->setToolTip(tr("Дублировать строку (Ctrl+R)"));
    m_groupCorrection->setToolTip(tr("Групповая корректировка"));
    m_actAutoFilter->setToolTip(tr("Показать/скрыть строку автофильтра"));

    m_actAutoFilter->setCheckable(true);
    m_actAutoFilter->setChecked(false);

    connect(m_actAddRow,        &QAction::triggered,
            this, &RtabWidget::slot_addRow);
    connect(m_actInsertRow,     &QAction::triggered,
            this, &RtabWidget::slot_insertRow);
    connect(m_actDeleteRow,     &QAction::triggered,
            this, &RtabWidget::slot_deleteRow);
    connect(m_actDuplicateRow,  &QAction::triggered,
            this, &RtabWidget::slot_duplicateRow);
    connect(m_groupCorrection,  &QAction::triggered,
            this, &RtabWidget::slot_groupCorrection);
    connect(m_actAutoFilter,    &QAction::toggled,
            this, &RtabWidget::slot_toggleAutoFilter);
}

void RtabWidget::setupShortcuts(){
    // Qt::WidgetWithChildrenShortcut + parent=m_grid:
    // шорткат срабатывает, когда фокус внутри m_grid или его дочерних виджетов.
    // Это именно то, что нужно: работает когда пользователь в таблице,
    // и НЕ конфликтует с другими открытыми формами.
    struct Def { QKeySequence key; void (RtabWidget::*slot)(); };
    const Def defs[] = {
                        { Qt::CTRL | Qt::Key_I, &RtabWidget::slot_insertRow    },
                        { Qt::CTRL | Qt::Key_A, &RtabWidget::slot_addRow       },
                        { Qt::CTRL | Qt::Key_R, &RtabWidget::slot_duplicateRow },
                        { Qt::CTRL | Qt::Key_D, &RtabWidget::slot_deleteRow    },
                        };

    for (const auto& d : defs) {
        auto* sc = new QShortcut(d.key, m_grid);
        sc->setContext(Qt::WidgetWithChildrenShortcut);
        connect(sc, &QShortcut::activated, this, d.slot);
    }
}

void RtabWidget::notifyParentRowChanged(int modelRow) {
    m_linkedFormCtrl->onParentRowChanged(modelRow);
}

void RtabWidget::closeEvent(QCloseEvent *event)
{
    spdlog::info("RtabWidget::closeEvent [{}]", m_UIForm.Name().c_str() );

    // Сначала — отключить все входящие сигналы к модели
    disconnect(m_pRTDM, nullptr, m_model.get(), nullptr);
    disconnect(m_pRTDM, nullptr, this, nullptr);

    // Контроллер отключает Qt-соединения связанных форм
    m_linkedFormCtrl->disconnectAll();
    // Освобождаем DataBlock — модель перестаёт держать shared_ptr
    m_model->getRdata()->pnparray_.reset();
    QWidget::closeEvent(event);
};

void RtabWidget::slot_close(){
    this->close();
}

void RtabWidget::createModel()
{
    m_model = std::make_unique<RModel>(this, m_pqastra, m_pRTDM);
    m_model->setForm(&m_UIForm);
    if (!m_model->populateDataFromRastr())
    {
        // Таблица не найдена в плагине (файл не загружен или имя неверно).
        // Модель пуста — показываем сообщение и прекращаем инициализацию.
        spdlog::error("RtabWidget: populateDataFromRastr failed for table [{}]",
                      m_UIForm.TableName());
        QMessageBox::warning(
            this,
            tr("Ошибка открытия таблицы"),
            tr("Таблица \"%1\" недоступна.\n"
               "Убедитесь, что файл данных загружен.")
                .arg(QString::fromStdString(m_UIForm.Name())));
        return;   // m_model валиден, но пуст — Grid не инициализируем
    }
    m_view->beginUpdate();
    m_view->setModel(m_model.get());

    applyAllColumnEditors();

    //Порядок колонок как в форме
    int vi = 0;
    for (const auto& f : m_UIForm.Fields()){
        for (const RCol& rcol : *m_model->getRdata()){
            if (f.Name() == rcol.getColName()){
                Qtitan::GridTableColumn* column_qt;
                column_qt = static_cast<GridTableColumn*>(
                    m_view->getColumn(rcol.getIndex()));
                column_qt->setVisualIndex(vi++);
                break;
            }
        }
    }
    m_view->endUpdate();

    // ── Применяем ширины из бэка при первом открытии ──
    // setTableView отключает columnAutoWidth и выставляет ширины из RCol::getWidth()
    if (!m_UIForm.Vertical()){
        setTableView();
    }

    m_linkedFormCtrl = std::make_unique<LinkedFormController>(
        m_pqastra,
        m_pRTDM,
        m_DockManager,
        m_view,
        m_model.get(),
        m_UIForm,
        this);

    m_condFormatCtrl = std::make_unique<CondFormatController>(
        m_model.get(), m_view, this);
    m_condFormatCtrl->loadFromJson();

    ///@todo проверить необходимость в этих сигналах
    this->update();
    this->repaint();
}

void RtabWidget::applyAllColumnEditors(){
    for (int i = 0; i < m_model->columnCount(); ++i)
        applyColumnEditor(i);
}

void RtabWidget::applyColumnEditor(int colIndex)
{
    auto* column_qt = static_cast<Qtitan::GridTableColumn*>(
        m_view->getColumn(colIndex));
    if (!column_qt) return;

    const RCol* col = m_model->getRCol(colIndex);
    if (!col) return;

    column_qt->setVisible(!col->isHidden());

    const auto info = m_model->getColumnEditorInfo(colIndex);

    switch (info.editorType)
    {
    case RModel::ColumnEditorInfo::Type::CheckBox:
        column_qt->setEditorType(GridEditor::CheckBox);
        static_cast<Qtitan::GridCheckBoxEditorRepository*>(
            column_qt->editorRepository())
            ->setAppearance(GridCheckBox::StyledAppearance);
        break;
    case RModel::ColumnEditorInfo::Type::Numeric: {
        column_qt->setEditorType(GridEditor::String);
        // При использовании GridEditor::Numeric не удаётся убрать кнопки виджета,
        //поэтому валидатор добавляется вручную
        auto* repo = static_cast<GridStringEditorRepository*>(
            column_qt->editorRepository());

        // QDoubleValidator::decimals ограничивает знаки при вводе
        auto* val = new QDoubleValidator(info.minVal, info.maxVal,
                                         info.decimals, repo);
        val->setNotation(QDoubleValidator::StandardNotation);
        repo->setValidator(val);

        // QTitan должен сортировать по EditRole (double),
        // а не по DisplayRole (QString вида "117.20").
        // Без этого "21.20" < "117.20" даёт неверный результат при сортировке,
        // потому что строковое сравнение идёт посимвольно ('2' > '1').
        GridModelDataBinding* binding = m_view->getDataBinding(column_qt);
        if (binding)
            binding->setSortRole(Qt::EditRole);
        break;
    }
    case RModel::ColumnEditorInfo::Type::DateTime: {
        column_qt->setEditorType(GridEditor::DateTime);
        auto* repo = static_cast<Qtitan::GridDateTimeEditorRepository*>(
            column_qt->editorRepository());
        repo->setDisplayFormat("dd.MM.yyyy HH:mm");
        // Отключаем popup-календарь — без него QDateTimeEdit показывает
        // спиннер для каждого поля (день, месяц, год, час, минута)
        repo->setCalendarPopup(false);
        break;
    }
    case RModel::ColumnEditorInfo::Type::Color:{
        column_qt->setEditorType(GridEditor::Color);
        break;
    }
    case RModel::ColumnEditorInfo::Type::ComboBox:
        column_qt->setEditorType(GridEditor::ComboBox);
        if (!info.comboItems.isEmpty()) {
            column_qt->editorRepository()->setDefaultValue(
                info.comboItems.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(
                info.comboItems,
                static_cast<Qt::ItemDataRole>(Qtitan::ComboBoxRole));
        }
        break;
    case RModel::ColumnEditorInfo::Type::NameRef: {
        auto* repo = new SearchableComboRepositoryTwo(info.nameRefData.items, m_grid);
        column_qt->setEditorRepository(repo);
        break;
    }
    case RModel::ColumnEditorInfo::Type::ComboBoxPicture:
    {
        // GridPictureComboBoxEditorRepository -- НЕВЕРНО, это редактор одной картинки
        // Используем обычный ComboBox — он умеет иконки через ComboBoxRole
        column_qt->setEditorType(GridEditor::ComboBox);
        // Если нужна кастомная ширина под иконки:
        auto* repo = static_cast<Qtitan::GridComboBoxEditorRepository*>(
            column_qt->editorRepository());
        repo->setComboBoxEditable(false);

        spdlog::info("applyColumnEditor ENPIC col={}, picItems={}", colIndex,
                     info.picItems.size());
        break;
    }
    case RModel::ColumnEditorInfo::Type::None:
    default:
        break;
    }
}

void RtabWidget::setTableView(int multiplier  )
{
    m_view->beginUpdate();
    m_view->tableOptions().setColumnAutoWidth(false);
    // Выравнивание
    for (auto [idx, width] : m_model->columnsWidth()) {
        m_view->getColumn(idx)->setWidth(width * multiplier);
        m_view->getColumn(idx)->setTextAlignment(Qt::AlignLeft);
    }
    m_view->endUpdate();
}

void RtabWidget::slot_contextMenu(ContextMenuEventArgs* args)
{
    const auto& hit    = args->hitInfo();
    const int   column = hit.columnIndex();

    if (column < 0) return; // клик вне колонок (пустое место справа)
    // ── Определяем тип области
    const auto type = hit.info();
    const bool isHeader =
        type == GridHitInfo::Column ||
        type == GridHitInfo::Band;

    // ── Меню заголовка
    if (isHeader) {
        RCol* col = m_model->getRCol(column);   // может быть nullptr — prepareForHeader это учитывает
        m_menuBuilder->prepareForHeader(column, col, args->contextMenu());
        return;
    }
    // ── Меню ячейки
    const int row = hit.row().rowIndex();
    const int row_model = hit.row().modelIndex().row();
    RCol* col = m_model->getRCol(column);
    if (!col) return;

    MenuContext ctx { column, row_model, col };
    m_menuBuilder->prepareForShow(ctx, args->contextMenu());
}

void RtabWidget::slot_focusRowChanged(int row_old, int row_new){
    m_linkedFormCtrl->onParentRowChanged(getModelFocuedRow());
}

int RtabWidget::getModelFocuedRow(){
    return m_view->focusedRowIndex();
}

int RtabWidget::getModelFocuedColumn(){
    return m_view->focusedColumnIndex();
}

void RtabWidget::slot_addRow()
{
    m_view->beginUpdate();
    m_model->addRow();
    m_view->endUpdate();

    int modelRow = m_model->rowCount() - 1;
    if (modelRow < 0) return;

    // Конвертируем модельный индекс → GridRow → визуальный индекс
    QModelIndex newIdx = m_model->index(modelRow, 0);
    GridRow newGridRow = m_view->getRow(newIdx);
    if (newGridRow.isValid()) {
        m_view->scrollToRow(newGridRow);
        m_view->setFocusedRowIndex(newGridRow.rowIndex());
    }
}

void RtabWidget::slot_insertRow()
{
    // cell().modelIndex() — правильно, возвращает QModelIndex
    int modelRow = m_view->selection()->cell().modelIndex().row();

    m_view->beginUpdate();
    m_model->insertRows(modelRow, 1);
    m_view->endUpdate();

    // Переводим фокус на вставленную строку через модельный индекс
    GridRow inserted = m_view->getRow(m_model->index(modelRow, 0));
    if (inserted.isValid())
        m_view->setFocusedRowIndex(inserted.rowIndex());
}

void RtabWidget::slot_duplicateRow()
{
    int modelRow = m_view->selection()->cell().modelIndex().row();

    m_view->beginUpdate();
    m_model->duplicateRow(modelRow);
    m_view->endUpdate();

    // Дубликат вставляется на позицию modelRow
    GridRow dup = m_view->getRow(m_model->index(modelRow, 0));
    if (dup.isValid())
        m_view->setFocusedRowIndex(dup.rowIndex());
}

void RtabWidget::slot_deleteRow()
{
    QModelIndexList selected = m_view->selection()->selectedRowIndexes();
    if (selected.isEmpty()) return;

    if (selected.count() > 1) {
        auto btn = QMessageBox::question(this,
                                         tr("Подтверждение"),
                                         tr("Удалить %1 записей?").arg(selected.count()),
                                         QMessageBox::Yes | QMessageBox::Cancel);
        if (btn != QMessageBox::Yes) return;
    }

    // Минимальный model row — начало диапазона удаления
    int minModelRow = selected.first().row();
    for (const QModelIndex& mi : selected)
        minModelRow = std::min(minModelRow, mi.row());

    m_view->beginUpdate();
    //Примем допущение что selection() один и не имеет разрывов
    m_model->removeRows(minModelRow, selected.count());
    m_view->endUpdate();

    // Фокус на строку, которая оказалась на месте удалённых
    int newFocus = std::min(minModelRow, m_model->rowCount() - 1);
    if (newFocus >= 0) {
        GridRow r = m_view->getRow(m_model->index(newFocus, 0));
        if (r.isValid())
            m_view->setFocusedRowIndex(r.rowIndex());
    }
}

void RtabWidget::slot_beginResetModel(std::string tname)
{
    if (m_UIForm.TableName() != tname) return;

    m_view->beginUpdate(); // ← открываем внешний блок

    // Сохраняем видимость по имени колонки (не по caption — он может меняться)
    m_columnsVisible.clear();
    for (const RCol& rcol : *m_model->getRdata()) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(
            m_view->getColumn(rcol.getIndex()));
        m_columnsVisible[QString::fromStdString(rcol.getColName())]
            = col ? col->isVisible() : true;
    }
}

void RtabWidget::slot_endResetModel(std::string tname)
{
    if (m_UIForm.TableName() != tname) return;

    // Восстанавливаем видимость и переназначаем редакторы.
    // К этому моменту RModel::slot_EndResetModel уже вызвал
    // populateDataFromRastr() — новые RData/RCol уже готовы.
    for (const RCol& rcol : *m_model->getRdata()) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(
            m_view->getColumn(rcol.getIndex()));
        if (!col) continue;

        // Восстанавливаем видимость
        auto it = m_columnsVisible.find(
            QString::fromStdString(rcol.getColName()));
        col->setVisible(it != m_columnsVisible.end() ? it->second : true);

        // Синхронизируем caption с обновлённым заголовком модели.
        // Qtitan кеширует caption независимо от headerData() — нужно
        // обновить вручную после сброса.
        QVariant title = m_model->headerData(
            rcol.getIndex(), Qt::Horizontal, Qt::DisplayRole);
        if (title.isValid())
            col->setCaption(title.toString());
    }

    // Пересоздаём репозитории — данные nameref
    // (например, для SearchableComboPopupTwo)  могли смениться
    m_view->beginUpdate();  //← открываем внутренний для applyAllColumnEditors
    applyAllColumnEditors();
    m_view->endUpdate();    //← закрываем внутренний

    m_view->endUpdate();     // ← закрываем внешний из slot_beginResetModel

    // После полного сброса модели очищаем автофильтр:
    // структура колонок могла измениться.
    if (m_autoFilter) {
        m_autoFilterCond->clearAll();
        m_autoFilter->clearAll();     // очищает поля и эмитит filterChanged→ rebuild
        m_autoFilter->rebuild();      // пересоздаёт ячейки под новые колонки
    }
    m_selectionFilter.clear();
    m_selection = "";
    rebuildCombinedFilter();
}

void RtabWidget::slot_groupCorrection()
{
    const int col = m_view->selection()->cell().columnIndex();
    RCol* prcol = m_model->getRCol(col);
    if (!prcol){
        return;
    }
    GroupCorrectionDialog* fgc =  new GroupCorrectionDialog(m_model->getRdata(),prcol,this);
    fgc->setAttribute(Qt::WA_DeleteOnClose);

    fgc->show();
}

void RtabWidget::slot_openColProp(int col)
{
    RCol* prcol = m_model->getRCol(col);
    if (!prcol) return;
    ColPropDialog* propDialog = new ColPropDialog(m_model->getRdata(),
                                                  m_view, prcol, this);
    propDialog->setAttribute(Qt::WA_DeleteOnClose);
    propDialog->exec();
}

void RtabWidget::slot_openSelection(int col)
{
    int column = getModelFocuedColumn();
    RCol* prcol = m_model->getRCol(col);
    std::string colName = prcol ? prcol->getColName() : "";

    auto* selectionDialog = new SelectionDialog(m_selection, colName, this);
    selectionDialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(selectionDialog, &SelectionDialog::sig_selectionAccepted,
            this, &RtabWidget::slot_setFiltrForSelection);

    // Закрываем диалог сразу после того, как пользователь принял выборку
    connect(selectionDialog, &SelectionDialog::sig_selectionAccepted,
            selectionDialog, &QDialog::close);

    selectionDialog->show();
}

void RtabWidget::slot_openExportCSVForm()
{
    ExportCSVdialog* exportCsvDialog = new ExportCSVdialog( m_model->getRdata(),this);
    exportCsvDialog->setAttribute(Qt::WA_DeleteOnClose);
    exportCsvDialog->show();
}

void RtabWidget::slot_openImportCSVForm()
{
    ImportCSV2dialog* importCsvDialog = new ImportCSV2dialog( m_model->getRdata(),this);
    importCsvDialog->setAttribute(Qt::WA_DeleteOnClose);
    importCsvDialog->show();
}

void RtabWidget::slot_directCodeToggle(std::size_t column)
{
    RCol* prcol = m_model->getRCol(column);
    if (!prcol) return;
    prcol->invertDirectCodeStatus();

    m_view->beginUpdate();
    applyColumnEditor(column);
    m_view->endUpdate();

    // Принудительно перерисовать всю колонку — DisplayRole изменился
    const int rows = m_model->rowCount();
    if (rows > 0) {
        emit m_model->dataChanged(
            m_model->index(0,      static_cast<int>(column)),
            m_model->index(rows-1, static_cast<int>(column)),
            {Qt::DisplayRole, Qt::EditRole});
    }
}

void RtabWidget::slot_condFormatsEdit(std::size_t column)
{
    m_condFormatCtrl->editCondFormats(column);
}

void RtabWidget::slot_widthByTemplate(){
    if (m_view != nullptr && m_model != nullptr){
        setTableView();
    }
}

void RtabWidget::slot_widthByData(){
    m_view->tableOptions().setColumnAutoWidth(true);
}

void RtabWidget::slot_setFiltrForSelection(std::string selection)
{
    m_selection    = selection;
    // сохраняем для rebuildCombinedFilter
    m_selectionFilter = QString::fromStdString(selection);
    rebuildCombinedFilter();
}

void RtabWidget::slot_toggleAutoFilter(bool checked)
{
    m_autoFilter->setVisible(checked);
    if (!checked) {
        // Сброс условий при скрытии
        m_autoFilter->clearAll();
        // clearAll уже испустит filterChanged → rebuildCombinedFilter
        // но для надёжности вызовем явно
        m_autoFilterCond->clearAll();
        rebuildCombinedFilter();
    }
}

void RtabWidget::slot_applyAutoFilter(int colIndex, const QString& text)
{
    FilterRule rule = FilterRuleParser::parse(text);
    if (rule.isActive())
        m_autoFilterCond->setRule(colIndex, rule);
    else
        m_autoFilterCond->clearRule(colIndex);

    rebuildCombinedFilter();
}

void RtabWidget::rebuildCombinedFilter()
{
    const bool hasSelection  = !m_selectionFilter.isEmpty();
    const bool hasAutoFilter = m_autoFilterCond && m_autoFilterCond->hasActiveRules();

    if (!hasSelection && !hasAutoFilter) {
        m_view->filter()->setActive(false);
        return;
    }

    auto* group = new Qtitan::GridFilterGroupCondition(m_view->filter());

    if (hasSelection) {
        // Воссоздаём CustomFilterCondition из m_selectionFilter.
        // Повторяем логику получения строк из плагина
        IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
        IRastrTablePtr  table{ tablesx->Item(m_model->getRdata()->t_name_) };
        IRastrResultVerify(table->SetSelection(m_selectionFilter.toStdString()));

        DataBlock<FieldVariantData> variantBlock;
        const IRastrPayload keys = table->Key();
        IRastrResultVerify(table->DataBlock(keys.Value(), variantBlock));
        const auto indices = variantBlock.IndexesVector();

        auto* selCond = new CustomFilterCondition(m_view->filter());
        for (long idx : indices)
            selCond->addRow(static_cast<int>(idx));
        group->addCondition(selCond);
    }

    if (hasAutoFilter) {
        // clone() — QTitan владеет копией, мы сохраняем оригинал для изменений
        group->addCondition(m_autoFilterCond->clone());
    }

    m_view->filter()->setCondition(group, true);
    m_view->filter()->setActive(true);
}