#include "rtabcontroller.h"

#include "rtabshell.h"
#include "filtermanager.h"
#include "rmodel.h"
#include "rdata.h"
#include "rcol.h"
#include "rgrid.h"
#include "rtablesdatamanager.h"
#include "linkedformcontroller.h"
#include "contextmenubuilder.h"
#include "condformatcontroller.h"
#include "tables/colPropDialog.h"
#include "tables/groupCorrectionDialog.h"
#include "tables/exportCSVdialog.h"
#include "tables/importCSV2dialog.h"
#include "customEditors/searchableComboEditorTwo/searchableComboRepositoryTwo.h"
#include <QContextMenuEvent>
#include "QtitanBase.h"
#include <QShortcut>
#include <QMessageBox>
#include <QCloseEvent>
#include "QDataBlocks.h"
#include <spdlog/spdlog.h>

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

RtabController::RtabController(QAstra*             pqastra,
                                CUIForm             UIForm,
                                RTablesDataManager* pRTDM,
                                ads::CDockManager*  pDockManager,
                                QObject*            parent)
    : QObject(parent)
    , m_UIForm(std::move(UIForm))
    , m_pRTDM(pRTDM)
    , m_DockManager(pDockManager)
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

    createModel(pqastra);

    m_menuBuilder = std::make_unique<ContextMenuBuilder>(
        m_view, m_linkedFormCtrl.get(), this);
    m_menuBuilder->initMenu(m_grid);

    m_filterManager = std::make_unique<FilterManager>(m_view, m_model.get(),
                                                      m_grid);

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

RtabController::~RtabController() = default;

RtabShell* RtabController::createShell(bool withToolbar)
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
        withToolbar,
        nullptr);   // владелец — CDockWidget, не контроллер

    // Статусбар shell'а обновляем при сбросе модели
    connect(m_model.get(), &QAbstractTableModel::modelReset,
            shell, &RtabShell::slot_updateStatusLabel);

    return shell;
}

void RtabController::setupShortcuts(RtabController* target, RGrid* grid)
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
        QObject::connect(sc, &QShortcut::activated, target, d.slot); // цель — контроллер
    }
}

void RtabController::createModel(QAstra* pqastra)
{
    m_model = std::make_unique<RModel>(this, pqastra, m_pRTDM);
    m_model->setForm(&m_UIForm);
    if (!m_model->populateDataFromRastr())
    {
        // Таблица не найдена в плагине (файл не загружен или имя неверно).
        // Модель пуста — показываем сообщение и прекращаем инициализацию.
        spdlog::error("RtabWidget: populateDataFromRastr failed for table [{}]",
                      m_UIForm.TableName());
        QMessageBox::warning(
            m_grid,
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
        pqastra,
        m_pRTDM,
        m_DockManager,
        m_view,
        m_model.get(),
        m_UIForm,
        this);

    m_condFormatCtrl = std::make_unique<CondFormatController>(
        m_model.get(), m_view, m_grid);
    m_condFormatCtrl->loadFromJson();
}

void RtabController::setupConnections()
{
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
    //RTablesDataManager -> RtabController
    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this,
            &RtabController::slot_beginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this,
            &RtabController::slot_endResetModel);
    //ContextMenuBuilder -> RtabController
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_addRow,
            this, &RtabController::slot_addRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_insertRow,
            this, &RtabController::slot_insertRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_deleteRow,
            this, &RtabController::slot_deleteRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_duplicateRow,
            this, &RtabController::slot_duplicateRow);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_groupCorrection,
            this, &RtabController::slot_groupCorrection);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_colProp,
            this, &RtabController::slot_openColProp);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_exportCsv,
            this, &RtabController::slot_openExportCSVForm);
    connect(m_menuBuilder.get(), &ContextMenuBuilder::sig_importCsv,
            this, &RtabController::slot_openImportCSVForm);
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
    // Обновление ссылочных справочников при изменении строк в других таблицах
    connect(m_pRTDM,  &RTablesDataManager::sig_ReferenceChanged,
            m_model.get(), &RModel::slot_RefTableChanged);
    //QTitan
    //Connect Grid's context menu handler.
    connect(m_view, &GridTableView::contextMenu,
            this, &RtabController::slot_contextMenu);
}

void RtabController::applyAllColumnEditors(){
    for (int i = 0; i < m_model->columnCount(); ++i)
        applyColumnEditor(i);
}

void RtabController::applyColumnEditor(int colIndex)
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

        // QTitan должен сортировать по UserRole (double),
        // а не по DisplayRole/EditRole (QString вида "117.20").
        // Без этого "21.20" < "117.20" даёт неверный результат при сортировке,
        // потому что строковое сравнение идёт посимвольно ('2' > '1').
        GridModelDataBinding* binding = m_view->getDataBinding(column_qt);
        if (binding)
            binding->setSortRole(Qt::UserRole);
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
    case RModel::ColumnEditorInfo::Type::ComboBox: {
        column_qt->setEditorType(GridEditor::ComboBox);

        column_qt->editorRepository()->setDefaultValue(QString(), Qt::EditRole);
        column_qt->editorRepository()->setDefaultValue(info.comboItems,
                                                       (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        break;
    }
    case RModel::ColumnEditorInfo::Type::NameRef: {
        auto* repo = new SearchableComboRepositoryTwo(info.nameRefData.items, m_grid);
        column_qt->setEditorRepository(repo);
        break;
    }
    case RModel::ColumnEditorInfo::Type::ComboBoxPicture:
    {
        column_qt->setEditorType(GridEditor::ComboBox);
        spdlog::info("applyColumnEditor ENPIC col={}, picItems={}", colIndex,
                     info.picItems.size());
        break;
    }
    case RModel::ColumnEditorInfo::Type::None:
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

void RtabController::slot_deleteRow(){
    QModelIndexList selected = m_view->selection()->selectedRowIndexes();
    if (selected.isEmpty()) return;

    // Собираем уникальные строки (selectedRowIndexes может дублировать при MultiCell)
    QSet<int> rowSet;
    for (const QModelIndex& mi : selected)
        rowSet.insert(mi.row());

    if (rowSet.size() > 1) {
        auto btn = QMessageBox::question(
            m_grid, tr("Подтверждение"),
            tr("Удалить %1 записей?").arg(rowSet.size()),
            QMessageBox::Yes | QMessageBox::Cancel);
        if (btn != QMessageBox::Yes) return;
    }

    // Сортируем по убыванию — удаляем снизу вверх,
    // чтобы индексы вышестоящих строк не смещались
    QList<int> rows(rowSet.begin(), rowSet.end());
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    m_view->beginUpdate();
    for (int row : rows)
        m_model->removeRows(row, 1);
    m_view->endUpdate();

    // Фокус на строку, ближайшую к удалённому диапазону
    const int topRow  = rows.last(); // наименьший (список убывающий)
    const int newFocus = std::min(topRow, m_model->rowCount() - 1);
    if (newFocus >= 0) {
        GridRow r = m_view->getRow(m_model->index(newFocus, 0));
        if (r.isValid())
            m_view->setFocusedRowIndex(r.rowIndex());
    }
}

void RtabController::slot_groupCorrection(){
    const int col = m_view->selection()->cell().columnIndex();
    RCol* prcol = m_model->getRCol(col);
    if (!prcol){
        return;
    }
    GroupCorrectionDialog* fgc =  new GroupCorrectionDialog(m_model->getRdata(),prcol,m_grid);
    fgc->setAttribute(Qt::WA_DeleteOnClose);

    fgc->show();
}

void RtabController::slot_beginResetModel(std::string tname){
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

void RtabController::slot_endResetModel(std::string tname){
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

    m_filterManager->resetAfterModelReset();
}

void RtabController::slot_close()
{
    spdlog::info("RtabController::slot_close [{}]", m_UIForm.Name());

    // RTDM → модель и контроллер
    if (m_pRTDM) {
        disconnect(m_pRTDM, nullptr, m_model.get(), nullptr);
        disconnect(m_pRTDM, nullptr, this, nullptr);
    }

    // Отключаем focusRowChanged → child
    if (m_linkedFormCtrl)
        m_linkedFormCtrl->disconnectAll();

    // Сбрасываем компоненты в порядке зависимостей
    m_condFormatCtrl.reset();
    m_menuBuilder.reset();
    m_filterManager.reset();

    if (m_model && m_model->getRdata())
        m_model->getRdata()->pnparray_.reset();

    m_linkedFormCtrl.reset();
    m_model.reset();

    m_grid = nullptr;
    m_view = nullptr;

    this->deleteLater();
}

void RtabController::setPyHlp(std::shared_ptr<PyHlp> pPyHlp){
    pPyHlp_ = pPyHlp;
    if (m_linkedFormCtrl){
        m_linkedFormCtrl->setPyHlp(pPyHlp);
    }
}

void RtabController::slot_openColProp(int col){
    RCol* prcol = m_model->getRCol(col);
    if (!prcol) return;
    auto* propDialog = new ColPropDialog(m_model->getRdata(),
                                                  m_view, prcol, m_grid);
    propDialog->setAttribute(Qt::WA_DeleteOnClose);
    propDialog->exec();
}

void RtabController::slot_directCodeToggle(std::size_t column)
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

void RtabController::slot_condFormatsEdit(std::size_t column){
    m_condFormatCtrl->editCondFormats(column);
}

void RtabController::setTableView(int multiplier  )
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
    ExportCSVdialog* dlg = new ExportCSVdialog( m_model->getRdata(),m_grid);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void RtabController::slot_openImportCSVForm()
{
    auto* dlg = new ImportCSV2dialog( m_model->getRdata(),m_grid);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void RtabController::slot_contextMenu(ContextMenuEventArgs* args)
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

int RtabController::getLongValue(const std::string& key, long row){
    int col = m_model->getRdata()->mCols_.at(key);
    return std::visit(ToLong(), m_model->getRdata()->pnparray_->Get(row,col));
}

void RtabController::applyLinkedFormFromController(const LinkedForm& lf){
    // Точка входа для родительского LinkedFormController:
    // он вызывает этот метод на дочернем RtabWidget.
    m_linkedFormCtrl->applyLinkedForm(lf);
}

void RtabController::notifyParentRowChanged(int modelRow) {
    m_linkedFormCtrl->onParentRowChanged(modelRow);
}