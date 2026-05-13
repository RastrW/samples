#include "rtabcontroller.h"
#include <QElapsedTimer>
#include "rtabshell.h"
#include "filterManager.h"
#include "rmodel.h"
#include "rdata.h"
#include "rcol.h"
#include "rGrid.h"
#include "table/linkedForm/linkedformcontroller.h"
#include "contextMenuBuilder.h"
#include "condFormatController.h"
#include "tables/colPropDialog.h"
#include "tables/groupCorrectionDialog.h"
#include "tables/exportCSVdialog.h"
#include "tables/importCSV2dialog.h"
#include "customEditors/searchableComboEditor/searchableComboRepository.h"
#include <QContextMenuEvent>
#include "QtitanBase.h"
#include <QShortcut>
#include <QMessageBox>
#include <QCloseEvent>
#include <spdlog/spdlog.h>
#include "table/ITableEvents.h"
#include "table/ITableRepository.h"
#include "customEditors/doubleEditor/doubleEditorRepository.h"

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

RtabController::RtabController( std::shared_ptr<ITableRepository> tables,
                                std::shared_ptr<ITableEvents>     tableEvents,
                                CUIForm             UIForm,
                                ads::CDockManager*  pDockManager,
                                TableDockManager*   tableDockManager,
                                QObject*            parent)
    : QObject(parent)
    , m_tables (tables)
    , m_tableEvents(tableEvents)
    , m_UIForm(std::move(UIForm))
    , m_DockManager(pDockManager)
    , m_tableDockManager(tableDockManager)
{
    //  Настройка QTitan Grid
    Grid::loadTranslation();
    m_grid = new RGrid(nullptr);

    if (m_UIForm.Vertical())
        m_grid->setViewType(Qtitan::Grid::TableViewVertical);
    else
        m_grid->setViewType(Qtitan::Grid::TableView);

    m_view = m_grid->view<Qtitan::GridTableView>();

    if (m_UIForm.Vertical()){
        m_view->tableOptions().setRowSizingEnabled(true);
        //Высота строки подстраивается под контент
        //m_view->options().setRowAutoHeight(true);
        m_view->options().setRowAutoHeight(false);
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
    m_view->options().setFilterAutoHide(false);
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
    ///@todo (при отладки текст мельтешит, может быть в Release лучше) Эффект ускорения при быстром скролле
    //m_view->options().setFastScrollEffect(true);
    //Сохраняет фокус на строке после сортировки/фильтрации
    //m_view->options().setKeepFocusedRow(true);
    ///@todo (не работает?) Кнопка быстрой настройки видимости колонок в заголовке
    m_view->tableOptions().setColumnsQuickCustomization(true);
    //Анимация подсветки при добавлении строки
    m_view->options().setNewRowHighlightEffect(Qtitan::HighlightEffect::FlashEffect);
    ///@todo (не работает?)
    //m_view->showNewRowEditor();
    ///@todo (не работает?)
    //m_view->options().setGroupSummaryPlace(GridViewOptions::GroupSummaryPlace::SummaryRowPlus);
    ///@note создание модели обязатель до меню!

    createModel(tables);

    m_filterManager = std::make_unique<FilterManager>(m_view, m_model.get(),
                                                      m_grid);

    if (!m_UIForm.Vertical()) {
        createCommonTableActions();
    }
    m_menuBuilder = std::make_unique<ContextMenuBuilder>(
        m_view, m_linkedFormCtrl.get(), m_comTabAct, this);
    m_menuBuilder->initMenu(m_grid, m_UIForm.Vertical());

    setupConnections();

    //dumpShortcuts(m_grid, "before clear");
    auto& acts = m_view->actions();

    if (m_UIForm.Vertical()) {
        // Для вертикальных — снимаем вообще все шорткаты QTitan
        for (auto it = acts.begin(); it != acts.end(); ++it)
            it.value()->setShortcut(QKeySequence());
    } else {
        // Для горизонтальных — только конфликтующие Del и F5
        if (acts.contains(Qtitan::GridViewBase::DeleteRowAction))
            acts[Qtitan::GridViewBase::DeleteRowAction]->setShortcut(QKeySequence());

        for (auto it = acts.begin(); it != acts.end(); ++it)
            if (it.value()->shortcut() == QKeySequence(Qt::Key_F5))
                it.value()->setShortcut(QKeySequence());
    }

    //dumpShortcuts(m_grid, "after clear");
}

RtabController::~RtabController()
{
    spdlog::info("RtabController::~RtabController [{}]", m_UIForm.DisplayName());

    if (m_tableEvents) {
        disconnect(m_tableEvents.get(), nullptr, m_model.get(), nullptr);
        disconnect(m_tableEvents.get(), nullptr, this, nullptr);
    }

    // Отсоединяем грид от модели ДО уничтожения модели.
    // Иначе GridModelController::modelDestroyed попытается обратиться
    // к PersistentRow в уже разрушающейся модели.
    if (m_view) {
        m_view->setModel(nullptr);
    }

    // unique_ptr-ы уничтожаются в порядке объявления:
    // m_model уничтожается последним среди данных
}

RtabShell* RtabController::createShell(const TableProperties& tabProp)
{
    if (m_shellCreated) {
        spdlog::error("RtabController::createShell called twice for table [{}]",
                      m_UIForm.TableName());
        return nullptr;
    }
    m_shellCreated = true;

    auto* shell = new RtabShell(
        m_grid, m_view, m_model.get(),
        m_filterManager.get(), this,
        tabProp,
        nullptr);   // владелец — CDockWidget, не контроллер

    // Статусбар shell'а обновляем при сбросе модели
    connect(m_model.get(), &QAbstractTableModel::modelReset,
            shell, &RtabShell::slot_updateStatusLabel);

    return shell;
}

void RtabController::createCommonTableActions()
{
    m_comTabAct.addRow = new QAction(
        QIcon(":/images/Rastr3_grid_addrow_16x16.png"),
        tr("Добавить строку (Ctrl+A)"), this);

    m_comTabAct.insertRow = new QAction(
        QIcon(":/images/Rastr3_grid_insrow_16x16.png"),
        tr("Вставить строку (Ctrl+I)"), this);

    m_comTabAct.deleteRow = new QAction(
        QIcon(":/images/Rastr3_grid_delrow_16x16.png"),
        tr("Удалить строку (Ctrl+D)"), this);

    m_comTabAct.duplicateRow = new QAction(
        QIcon(":/images/Rastr3_grid_duprow_16x161.png"),
        tr("Дублировать строку (Ctrl+R)"), this);

    m_comTabAct.groupCorr = new QAction(
        QIcon(":/images/column_edit.png"),
        tr("Групповая корректировка"), this);

    connect(m_comTabAct.addRow,       &QAction::triggered,
            this, &RtabController::slot_addRow);
    connect(m_comTabAct.insertRow,    &QAction::triggered,
            this, &RtabController::slot_insertRow);
    connect(m_comTabAct.deleteRow,    &QAction::triggered,
            this, &RtabController::slot_deleteRow);
    connect(m_comTabAct.duplicateRow, &QAction::triggered,
            this, &RtabController::slot_duplicateRow);
    connect(m_comTabAct.groupCorr,    &QAction::triggered,
            this, &RtabController::slot_groupCorrection);
}

void RtabController::setupShortcuts(RGrid* grid)
{
    // Qt::WidgetWithChildrenShortcut + parent=m_grid:
    // шорткат срабатывает, когда фокус внутри m_grid или его дочерних виджетов.
    // Это именно то, что нужно: работает когда пользователь в таблице,
    // и НЕ конфликтует с другими открытыми формами.
    struct Def { QKeySequence key; void (RtabController::*slot)(); };
    const Def defs[] = {
        { Qt::CTRL | Qt::Key_I, &RtabController::slot_insertRow    },
        { Qt::CTRL | Qt::Key_A, &RtabController::slot_addRow       },
        { Qt::CTRL | Qt::Key_R, &RtabController::slot_duplicateRow },
        { Qt::CTRL | Qt::Key_D, &RtabController::slot_deleteRow    },
    };
    for (const auto& d : defs) {
        auto* sc = new QShortcut(d.key, grid); // parent — grid (дочерний к shell)
        sc->setContext(Qt::WidgetWithChildrenShortcut); // фокус внутри grid
        QObject::connect(sc, &QShortcut::activated, this, d.slot); // цель — контроллер
    }
}

void RtabController::createModel(std::shared_ptr<ITableRepository> tables)
{
    QElapsedTimer t; t.start();

    m_model = std::make_unique<RModel>(this, tables);
    m_model->setForm(&m_UIForm);

    if (!m_model->populateDataFromRastr()) {
        // Таблица не найдена в плагине (файл не загружен или имя неверно).
        // Модель пуста — показываем сообщение и прекращаем инициализацию.
        spdlog::error("RtabController: populateDataFromRastr failed [{}]",
                      m_UIForm.TableName());
        QMessageBox::warning(m_grid, tr("Ошибка открытия таблицы"),
                             tr("Таблица \"%1\" недоступна.\nУбедитесь, что файл данных загружен.")
                                 .arg(QString::fromStdString(m_UIForm.DisplayName())));
        return;
    }

    m_view->beginUpdate();
    m_view->setModel(m_model.get());
    setupColumnOrder();

    if (!m_UIForm.Vertical())
        setTableView(false);

    applyAllColumnEditors();
    restoreColumnVisibility(m_model->getRdata());

    m_view->endUpdate();

    createLinkedFormController(tables);
    createCondFormatController();
    spdlog::debug("[PERF] createModel: {} ms", t.restart());
}

void RtabController::setupColumnOrder()
{
    // Расставляем визуальные индексы по порядку из формы
    VisualIndex vi{0};
    for (const auto& field : m_UIForm.Fields()) {
        const ModelIndex pos = m_model->getRdata().modelIndexOf(field.Name());
        if (pos.invalid()) continue;
        if (auto* col = columnByModelIndex(pos))
            col->setVisualIndex(vi.value++);
    }
}

void RtabController::createLinkedFormController
    (std::shared_ptr<ITableRepository> tables){

    m_linkedFormCtrl = std::make_unique<LinkedFormController>(
        tables, m_tableEvents, m_tableDockManager,
        m_view, m_model.get(), m_UIForm, this);
}

void RtabController::createCondFormatController(){

    m_condFormatCtrl = std::make_unique<CondFormatController>(
        m_model.get(), m_view, m_grid);
    m_condFormatCtrl->loadFromJson();
}

void RtabController::setupConnections()
{
    auto* ev = m_tableEvents.get();
    // BeginResetModel: СНАЧАЛА контроллер (сохраняет порядок),
    //                  ПОТОМ модель (вызывает beginResetModel, инвалидирует биндинги)
    connect(ev, &ITableEvents::sig_BeginResetModel, this,
            &RtabController::slot_beginResetModel);
    connect(ev, &ITableEvents::sig_BeginResetModel, m_model.get(),
            &RModel::slot_BeginResetModel);
    // EndResetModel:   СНАЧАЛА модель (populateDataFromRastr + endResetModel,
    //                              QTitan синхронизирует биндинги со старыми колонками),
    //                  ПОТОМ контроллер (removeColumns + addColumn с новым порядком)
    connect(ev, &ITableEvents::sig_EndResetModel, m_model.get(),
            &RModel::slot_EndResetModel);
    connect(ev, &ITableEvents::sig_EndResetModel, this,
            &RtabController::slot_endResetModel);

    //ITableEvents -> RModel
    connect(ev, &ITableEvents::sig_dataChanged,this->m_model.get(),
            &RModel::slot_DataChanged);
    connect(ev, &ITableEvents::sig_BeginInsertRow,this->m_model.get(),
            &RModel::slot_BeginInsertRow);
    connect(ev, &ITableEvents::sig_EndInsertRow,this->m_model.get(),
            &RModel::slot_EndInsertRow);
    connect(ev, &ITableEvents::sig_BeginRemoveRows,this->m_model.get(),
            &RModel::slot_BeginRemoveRows);
    connect(ev, &ITableEvents::sig_EndRemoveRows,this->m_model.get(),
            &RModel::slot_EndRemoveRows);
    //ContextMenuBuilder -> RtabController
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_exportCsv,
            this, &RtabController::slot_openExportCSVForm);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_importCsv,
            this, &RtabController::slot_openImportCSVForm);
    // Остальные сигналы ContextMenuBuilder — только для горизонтальных
    if (!m_UIForm.Vertical()) {
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_colProp,
                this, &RtabController::slot_openColProp);
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_widthByTemplate,
                this, &RtabController::slot_widthByTemplate);
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_widthByData,
                this, &RtabController::slot_widthByData);
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_directCodeToggle,
                this, &RtabController::slot_directCodeToggle);
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_condFormatsEdit,
                this, &RtabController::slot_condFormatsEdit);
        connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_selection,
                m_filterManager.get(), &FilterManager::slot_openSelection);
    }
    // Обновление ссылочных справочников при изменении строк в других таблицах
    connect(ev,  &ITableEvents::sig_ReferenceChanged,
            m_model.get(), &RModel::slot_RefTableChanged);
    connect(m_model.get(), &RModel::sig_nameRefUpdated,
            this, [this](const std::vector<ModelIndex>& cols) {
                for (const auto i : cols) {
                    auto* column_qt = columnByModelIndex(i);
                    if (!column_qt) continue;
                    auto* repo = static_cast<SearchableComboRepository*>(
                        column_qt->editorRepository());
                    if (!repo) continue;
                    // getColumnEditorInfo теперь возвращает const& — копии нет
                    const auto& info = m_model->getColumnEditorInfo(i);
                    repo->updateItems(info.nameRefData);
                }
            });
    if (!m_UIForm.Vertical()){
        connect(m_view, &GridTableView::contextMenu,
                this, &RtabController::slot_contextMenu);
    }else{
        connect(m_view, &GridTableView::contextMenu,
                this, &RtabController::slot_contextMenuVertical);
    }
}

void RtabController::applyAllColumnEditors()
{
    const int count = m_view->getColumnCount();
    for (int listIdx = 0; listIdx < count; ++listIdx) {
        applyColumnEditor(modelIndexFromListIndex(listIdx));
    }
}

void RtabController::applyColumnEditor(ModelIndex col)
{
    auto* column_qt = columnByModelIndex(col);
    if (!column_qt) return;

    const RCol* rcol = m_model->getRCol(col);
    if (!rcol) return;

    // Проверяем целевую видимость из сохранённого состояния,
    // а не текущую column_qt->isVisible() — она ещё не восстановлена.
    const QString qname = QString::fromStdString(rcol->getColName());
    const auto it = m_columnState.find(qname);
    const bool targetVisible = (it != m_columnState.end())
                                   ? it->second.visible
                                   : !rcol->isHiddenByForm();
    // Редакторы не настраиваем для скрытых колонок:
    // QTitan при setEditorType/setEditorRepository вызывает data() на ячейках,
    // что провоцирует lazy-load для незагруженных колонок.
    // При показе колонки applyColumnEditor будет вызван повторно.
    if (!targetVisible) return;

    column_qt->setProperty("isNumeric", false);
    column_qt->setProperty("isBool",    false);

    const auto& info = m_model->getColumnEditorInfo(col);

    switch (info.editorType)
    {
    case ColumnEditorInfo::Type::CheckBox:
        column_qt->setProperty("isBool", true);
        column_qt->setEditorType(GridEditor::CheckBox);
        static_cast<Qtitan::GridCheckBoxEditorRepository*>(
            column_qt->editorRepository())
            ->setAppearance(GridCheckBox::StyledAppearance);
        break;

    case ColumnEditorInfo::Type::Numeric: {
        column_qt->setProperty("isNumeric", true);
        column_qt->setEditorRepository(
            new DoubleEditorRepository(info.decimals, info.minVal, info.maxVal));
        if (auto* binding = m_view->getDataBinding(column_qt))
            binding->setSortRole(Qt::UserRole);
        break;
    }
    case ColumnEditorInfo::Type::DateTime: {
        column_qt->setEditorType(GridEditor::DateTime);
        auto* repo = static_cast<Qtitan::GridDateTimeEditorRepository*>(
            column_qt->editorRepository());
        repo->setDisplayFormat("dd.MM.yyyy HH:mm");
        // Отключаем popup — без него QDateTimeEdit показывает спиннер по полю.
        repo->setCalendarPopup(false);
        break;
    }
    case ColumnEditorInfo::Type::Color:
        column_qt->setEditorType(GridEditor::Color);
        break;

    case ColumnEditorInfo::Type::ComboBox:
        column_qt->setProperty("isNumeric", true);
        column_qt->setEditorType(GridEditor::ComboBox);
        column_qt->editorRepository()->setDefaultValue(QString(), Qt::EditRole);
        column_qt->editorRepository()->setDefaultValue(
            info.comboItems, static_cast<Qt::ItemDataRole>(Qtitan::ComboBoxRole));
        break;

    case ColumnEditorInfo::Type::NameRef:
        column_qt->setProperty("isNumeric", true);
        column_qt->setEditorRepository(
            new SearchableComboRepository(info.nameRefData, m_grid));
        break;

    case ColumnEditorInfo::Type::ComboBoxPicture:
        column_qt->setProperty("isNumeric", true);
        column_qt->setEditorType(GridEditor::ComboBox);
        break;

    case ColumnEditorInfo::Type::None:
    default:
        break;
    }
}

void RtabController::slot_addRow(){

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

void RtabController::slot_insertRow(){

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

void RtabController::slot_duplicateRow(){

    int modelRow = m_view->selection()->cell().modelIndex().row();

    m_view->beginUpdate();
    m_model->duplicateRow(modelRow);
    m_view->endUpdate();

    // Дубликат вставляется на позицию modelRow
    GridRow dup = m_view->getRow(m_model->index(modelRow, 0));
    if (dup.isValid())
        m_view->setFocusedRowIndex(dup.rowIndex());
}

void RtabController::slot_deleteRow()
{
    QModelIndexList selected = m_view->selection()->selectedRowIndexes();
    if (selected.isEmpty()) return;

    // std::set<int, greater> — сразу в порядке убывания,
    // чтобы индексы вышестоящих строк не смещались
    std::set<int, std::greater<int>> rows;
    for (const QModelIndex& mi : selected)
        rows.insert(mi.row());

    if (rows.size() > 1) {
        auto btn = QMessageBox::question(
            m_grid, tr("Подтверждение"),
            tr("Удалить %1 записей?").arg(rows.size()),
            QMessageBox::Yes | QMessageBox::Cancel);
        if (btn != QMessageBox::Yes) return;
    }

    m_view->beginUpdate();
    for (int row : rows)
        m_model->removeRows(row, 1);
    m_view->endUpdate();

    // Фокус на строку, ближайшую к удалённому диапазону
    const int minRow  = *rows.rbegin(); // наименьший индекс
    const int newFocus = std::min(minRow, m_model->rowCount() - 1);
    if (newFocus >= 0) {
        GridRow r = m_view->getRow(m_model->index(newFocus, 0));
        if (r.isValid())
            m_view->setFocusedRowIndex(r.rowIndex());
    }
}

void RtabController::slot_groupCorrection(){

    const auto* prcol = m_model->getRCol
        (modelIndexFromListIndex(m_view->selection()->cell().columnIndex()));
    if (!prcol){return;}

    GroupCorrectionDialog* fgc =
        new GroupCorrectionDialog(m_tables,
                                  m_model->getRdata(),prcol,m_grid);
    fgc->setAttribute(Qt::WA_DeleteOnClose);

    fgc->show();
}

void RtabController::slot_beginResetModel(const std::string& tname)
{
    if (m_UIForm.TableName() != tname) return;
    QElapsedTimer t; t.start();

    m_columnState.clear();

    // getColumnCount() возвращает все добавленные через addColumn(),
    // в т.ч. скрытые через setVisible(false).
    const int count = m_view->getColumnCount();
    const RData& rdata = m_model->getRdata();

    for (int listIdx = 0; listIdx < count; ++listIdx) {
        auto* gridCol = m_view->getColumn(listIdx);
        if (!gridCol) continue;
        const RCol* rcol = rdata.colAt(modelIndexFromListIndex(listIdx));
        if (!rcol) continue;

        const QString qname = QString::fromStdString(rcol->getColName());
        m_columnState[qname] = {gridCol->isVisible(), VisualIndex{gridCol->visualIndex()}};
    }

    spdlog::debug("[PERF] slot_beginResetModel: {} ms", t.restart());
}

void RtabController::slot_endResetModel(const std::string& tname)
{
    if (m_UIForm.TableName() != tname) return;
    QElapsedTimer t; t.start();
    const RData& rdata = m_model->getRdata();
    //Сбрасываем мендеджер фильтрации, т.к. он будет излишне перестраиваться
    //при изменении видимости колонок постоянно
    m_filterManager.reset();

    m_view->beginUpdate();
    m_view->setModel(m_model.get());
    restoreColumnOrder(rdata);

    if (!m_UIForm.Vertical())
        setTableView(false);
    applyAllColumnEditors();
    restoreColumnVisibility(rdata);

    m_view->endUpdate(); // layout строится один раз, только для видимых колонок

    m_filterManager = std::make_unique<FilterManager>(m_view, m_model.get(),
                                                      m_grid);
    spdlog::debug("[PERF] slot_endResetModel: {} ms", t.restart());
}

void RtabController::restoreColumnOrder(const RData& rdata)
{
    const int totalCols = static_cast<int>(rdata.size());

    // Шаг 1: собрать все (name → желаемый VisualIndex) из сохранённого состояния,
    // отфильтровав только те, что реально есть в новом rdata
    std::vector<std::pair<ModelIndex /*modelIdx*/, int /*savedVI*/>> known;
    std::vector<ModelIndex> unknown; // modelIdx без сохранённого порядка

    for (int i = 0; i < totalCols; ++i) {
        const RCol* rcol = rdata.colAt(ModelIndex{i});
        if (!rcol) continue;
        const QString qname = QString::fromStdString(rcol->getColName());
        auto it = m_columnState.find(qname);
        if (it != m_columnState.end() && it->second.vi.value >= 0)
            known.push_back({ModelIndex{i}, it->second.vi.value});
        else
            unknown.push_back(ModelIndex{i});
    }

    // Шаг 2: нормализовать known — отсортировать по сохранённому VI,
    // переназначить плотные индексы 0, 1, 2, ...
    std::sort(known.begin(), known.end(),
              [](const auto& a, const auto& b){ return a.second < b.second; });

    int vi = 0;
    for (const auto& [modelIdx, _] : known) {
        auto* col = columnByModelIndex(modelIdx);
        if (col) col->setVisualIndex(vi++);
    }
    // Шаг 3: новые колонки — в конец
    for (const auto& modelIdx : unknown) {
        auto* col = columnByModelIndex(modelIdx);
        if (col) col->setVisualIndex(vi++);
    }
}

void RtabController::restoreColumnVisibility(const RData& rdata)
{
    QElapsedTimer t; t.start();
    for (int i = 0; i < static_cast<int>(rdata.size()); ++i) {
        const ModelIndex col{i};
        auto* gridCol = columnByModelIndex(col);
        if (!gridCol) continue;
        const RCol* rcol = rdata.colAt(col);
        if (!rcol) continue;
        const QString qname = QString::fromStdString(rcol->getColName());
        auto it = m_columnState.find(qname);
        const bool isVisible = (it != m_columnState.end())
                                   ? it->second.visible
                                   : !rcol->isHiddenByForm();
        // Guard от лишних вызовов: после setModel все колонки visible=true.
        bool gridVis = gridCol->isVisible();
        if ( gridVis != isVisible){
            gridCol->setVisible(isVisible);
        }
    }
}

void RtabController::setPyHlp(std::shared_ptr<PyHlp> pPyHlp){
    if (m_linkedFormCtrl){
        m_linkedFormCtrl->setPyHlp(pPyHlp);
    }
}

void RtabController::slot_openColProp(ModelIndex col)
{
    // Получаем схему колонки через репозиторий
    const auto schema_full = m_tables->getSchema(m_UIForm.TableName());
    if (!col.valid_in(schema_full.columns.size())) return;

    const RCol* rcol = m_model->getRCol(col);
    if (!rcol) return;

    const auto it = std::find_if(
        schema_full.columns.begin(), schema_full.columns.end(),
        [&](const auto& cs){ return cs.name == rcol->getColName(); });

    if (it == schema_full.columns.end()) return;
    const auto& colSchema = *it;

    auto* dlg = new ColPropDialog(m_tables, colSchema, m_view, m_grid);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}

void RtabController::slot_directCodeToggle(ModelIndex column)
{
    m_model->invertDirectCode(column);

    m_view->beginUpdate();
    applyColumnEditor(ModelIndex{column});
    m_view->endUpdate();

    // Принудительно перерисовать всю колонку — DisplayRole изменился
    const int rows = m_model->rowCount();
    if (rows > 0) {
        emit m_model->dataChanged(
            m_model->index(0,      column.value),
            m_model->index(rows-1, column.value),
            {Qt::DisplayRole, Qt::EditRole});
    }
}

void RtabController::slot_condFormatsEdit(ModelIndex column){
    m_condFormatCtrl->editCondFormats(column);
}

void RtabController::setTableView(bool update, int multiplier)
{
    if (update) m_view->beginUpdate();
    m_view->tableOptions().setColumnAutoWidth(false);

    const int count = m_view->getColumnCount();
    for (int i = 0; i < count; ++i) {
        const RCol* rcol = m_model->getRCol(modelIndexFromListIndex(i) );
        if (!rcol || rcol->isHiddenByForm()) continue;
        int width = 10;
        try { width = std::stoi(rcol->getWidth()); }
        catch (...) {}
        auto* col = m_view->getColumn(i);
        col->setWidth(width * multiplier);
        col->setTextAlignment(Qt::AlignLeft);
    }

    if (update) m_view->endUpdate();
}

void RtabController::slot_widthByTemplate(){
    if (m_view != nullptr && m_model != nullptr){
        setTableView();
    }
}

void RtabController::slot_widthByData(){
    m_view->tableOptions().setColumnAutoWidth(true);
}

void RtabController::slot_toggleAutoFilter(bool checked){
    m_filterManager->toggle(checked);
}

void RtabController::slot_applyAutoFilter(int colIndex, const FilterRule& rule){
    m_filterManager->applyRule(colIndex, rule);
}

void RtabController::slot_setFiltrForSelection(std::string selection){
    m_filterManager->setSelection(selection);
}

void RtabController::slot_openExportCSVForm()
{
    std::string visCols = getVisibleColsFromGrid(); //Только видимые
    auto* dlg = new ExportCSVdialog(m_tables, m_UIForm.TableName(), visCols, m_grid);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void RtabController::slot_openImportCSVForm()
{
    std::string visCols = getVisibleColsFromGrid(); //Только видимые
    auto* dlg = new ImportCSV2dialog(
        m_tables,
        m_UIForm.TableName(),
        visCols,
        m_grid);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void RtabController::slot_contextMenu(ContextMenuEventArgs* args)
{
    const auto& hit = args->hitInfo();
    // клик вне колонок (пустое место справа)
    if (hit.columnIndex() < 0) return;

    // Получаем GridColumnBase через view и columnIndex
    GridViewBase* view = hit.view();
    if (!view) return;
    GridColumnBase* gridColBase = view->getColumn(hit.columnIndex());
    if (!gridColBase) return;

    auto* binding = view->getDataBinding(gridColBase);
    if (!binding) return;
    ModelIndex col {binding->column()};

    // Определяем тип области
    const auto type = hit.info();
    const bool isHeader = (type == GridHitInfo::Column || type == GridHitInfo::Band);

    // Меню заголовка
    if (isHeader) {
        const auto* rcol = m_model->getRCol(col);
        m_menuBuilder->prepareForHeader(col, rcol, args->contextMenu());
        return;
    }

    // Меню ячейки
    const int row_model = hit.row().modelIndex().row();
    const auto* rcol = m_model->getRCol(col);
    if (!rcol) return;

    MenuContext ctx {col, row_model, rcol };
    m_menuBuilder->prepareForShow(ctx, args->contextMenu());
}

void RtabController::slot_contextMenuVertical(ContextMenuEventArgs* args)
{
    QMenu* menu = args->contextMenu();
    menu->clear();
    menu->addAction(m_menuBuilder->actionExport());
    menu->addAction(m_menuBuilder->actionImport());
}

int RtabController::getLongValue(const std::string& key, long row){
    auto col = m_model->getRdata().mCols_.at(key);
    return std::visit(ToLong(), m_model->getRdata().getCell(col, row));
}

void RtabController::clearLinkedFilter()
{
    if (!m_linkedFormCtrl) return;
    m_linkedFormCtrl->clearFilter();
}

void RtabController::applyLinkedFormFromController(const LinkedForm& lf){
    // Точка входа для родительского LinkedFormController:
    // он вызывает этот метод на дочернем RtabWidget.
    m_linkedFormCtrl->applyLinkedForm(lf);
}

void RtabController::notifyParentRowChanged(int modelRow) {
    m_linkedFormCtrl->onParentRowChanged(modelRow);
}

Qtitan::GridTableColumn*
RtabController::columnByModelIndex(ModelIndex pos)const
{
    ///@note проверка диапазона предусмотрена внутри функции
    return qobject_cast<Qtitan::GridTableColumn*>(
        m_view->getColumnByModelColumn(pos.value));
}

ModelIndex RtabController::modelIndexOf(const std::string& colName) const
{
    if (!m_model) return {};
    const RData& rd = m_model->getRdata();
    auto it = rd.mCols_.find(colName);
    return (it != rd.mCols_.end()) ? it->second : ModelIndex{};
}

ModelIndex RtabController::modelIndexFromListIndex(int listIdx) const {
    auto* col = m_view->getColumn(listIdx);
    if (!col) return {};
    auto* binding = m_view->getDataBinding(col);
    if (!binding || binding->column() < 0) return {};
    return ModelIndex{binding->column()};
}

std::string
RtabController::getVisibleColsFromGrid() const
{
    std::string result;
    const int count = m_view->getColumnCount();
    for (int i = 0; i < count; ++i) {
        auto* gridCol = m_view->getColumn(i);
        if (!gridCol || !gridCol->isVisible()) continue;
        const RCol* rcol = m_model->getRCol(modelIndexFromListIndex(i));
        if (!rcol) continue;
        if (!result.empty()) result += ',';
        result += rcol->getColName();
    }
    return result;
}