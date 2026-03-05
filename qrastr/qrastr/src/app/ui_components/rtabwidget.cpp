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
#include "linkedform.h"

#include <QtitanGrid.h>
#include <utils.h>
#include <QAbstractItemModelTester>
#include <DockManager.h>
#include <QCloseEvent>
#include <QTableView>
#include <QMessageBox>

#include "formselection.h"
#include "formgroupcorrection.h"
#include "formexportcsv.h"
#include "formimportcsv2.h"
#include "qmcr/pyhlp.h"
#include <QToolBar>
#include "linkedformcontroller.h"
#include "ColPropForm.h"
#include "contextMenuBuilder.h"
#include "customFilterCondition.h"
#include "condFormatController.h"
#include "rmodel.h"

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
        m_grid->setViewType(Qtitan::Grid::TableViewVertical);
    }else{
        m_grid->setViewType(Qtitan::Grid::TableView);
    }

    m_view = m_grid->view< Qtitan::GridTableView>();
    m_view->options().setGridLines(Qtitan::LinesBoth);
    m_view->options().setGridLineWidth(1);
    m_view->tableOptions().setColumnAutoWidth(true);
    //user can select several cells at time. Hold shift key to select multiple cells.
    m_view->options().setSelectionPolicy(GridViewOptions::MultiCellSelection);
    m_view->options().setColumnHidingOnGroupingEnabled(false);
    // Sets the value that indicates whether the filter panel can automatically hide or not.
    m_view->options().setFilterAutoHide(true);
    // Sets the painting the doted frame around the cell with focus to the enabled. By default frame is enabled.
    m_view->options().setFocusFrameEnabled(true);
    // Sets the visibility status of the grid grouping panel to groupsHeader.
    m_view->options().setGroupsHeader(false);
    m_view->options().setScrollRowStyle(Qtitan::ScrollItemStyle::ScrollByItem);
    // Enables or disables wait cursor if grid is busy for lengthy operations with data like sorting or grouping.
    m_view->options().setShowWaitCursor(true);
    m_view->options().setRubberBandSelection(true);        // Выделение "резинкой"
    m_view->tableOptions().setColumnsHeader(true);
    m_view->tableOptions().setRowsQuickSelection(true);
    ///@todo Вынести в опцию контекстного меню (example MultiSelection)
    m_view->tableOptions().setRowFrozenButtonVisible(true);
    m_view->tableOptions().setFrozenPlaceQuickSelection(true);
    //отключить встроенное меню Qtitan
    //m_view->options().setMainMenuDisabled(true);

    // ── Блокируем встроенные в Qtitan события ──────────────────────────────────
    m_grid->installEventFilter(this);
    if (m_grid->viewport())
        m_grid->viewport()->installEventFilter(this);

    //  Горячие клавиши
    setupShortcuts();

    createModel();

    m_menuBuilder = std::make_unique<ContextMenuBuilder>(
        m_view,
        m_linkedFormCtrl.get(),
        this);
    m_menuBuilder->initMenu(this);

    setupConnections();

    resize(800,500);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMinMaxButtonsHint
                   | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::ApplicationModal);

    //qApp->installEventFilter(this);
}

RtabWidget::~RtabWidget() {
    //qApp->removeEventFilter(this);
}

QWidget* RtabWidget::createDockContent(bool addToolbar) {
    QWidget* wrapper = new QWidget(this);
    auto*    layout  = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (addToolbar) {
        setupToolbar();
        layout->addWidget(m_toolbar);
    }
    layout->addWidget(m_grid);
    return wrapper;
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
    connect(m_view, &GridTableView::cellClicked, this, &RtabWidget::slot_itemPressed);
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
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbar->setIconSize(QSize(16, 16));

    // Qtitan::Grid имеет встроенную кнопку поиска/фильтрации
    // Операции с данными
    m_actAddRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"), "Добавить");
    m_actInsertRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"), "Вставить");
    m_actDeleteRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"), "Удалить");
    m_actDuplicateRow = m_toolbar->addAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"), "Дублировать");
    m_groupCorrection = m_toolbar->addAction(QIcon(":/images/column_edit.png"), "Групповая коррекция");

    connect(m_actAddRow,       &QAction::triggered, this, &RtabWidget::slot_addRow);
    connect(m_actInsertRow,    &QAction::triggered, this, &RtabWidget::slot_insertRow);
    connect(m_actDeleteRow,    &QAction::triggered, this, &RtabWidget::slot_deleteRow);
    connect(m_actDuplicateRow, &QAction::triggered, this, &RtabWidget::slot_duplicateRow);
    connect(m_groupCorrection, &QAction::triggered, this, &RtabWidget::slot_groupCorrection);
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

bool RtabWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        // Срабатываем только если событие пришло от нашего grid или viewport
        bool fromGrid = (obj == m_grid)
                        || (m_grid && m_grid->isAncestorOf(qobject_cast<QWidget*>(obj)));
        if (fromGrid) {
            auto* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Delete ||
                ke->key() == Qt::Key_F5) {
                // Поглощаем событие
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void RtabWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "RtabWidget::closeEvent [" << m_UIForm.Name().c_str() << "]";

    // Контроллер отключает Qt-соединения связанных форм
    m_linkedFormCtrl->disconnectAll();

    // Освобождаем DataBlock — модель перестаёт держать shared_ptr
    m_model->getRdata()->pnparray_.reset();

    QWidget::closeEvent(event);
};

void RtabWidget::slot_close()
{
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
        //spdlog::error("RtabWidget: populateDataFromRastr failed for table [{}]",
        //              m_UIForm.TableName());
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

    //Show button menu for all column headers.
    //for (int i = 0; i < view->getColumnCount(); ++i)
    //    static_cast<GridTableColumn *>(view->getColumn(i))->setMenuButtonVisible(true);

    m_view->endUpdate();

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

    ///@todo проверить необходимо в этих сигналах
    this->update();
    this->repaint();
}

void RtabWidget::applyAllColumnEditors()
{
    m_view->beginUpdate();
    for (int i = 0; i < m_model->columnCount(); ++i)
        applyColumnEditor(i);
    m_view->endUpdate();
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
        column_qt->setEditorType(GridEditor::Numeric);
        auto* repo = static_cast<GridNumericEditorRepository*>(
            column_qt->editorRepository());
        repo->setMinimum(info.minVal);
        repo->setMaximum(info.maxVal);
        repo->setDecimals(info.decimals);
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

    case RModel::ColumnEditorInfo::Type::None:
    default:
        break;
    }
}

void RtabWidget::on_calc_begin()
{
    ///@todo something
    //view->beginUpdate();
}

void RtabWidget::on_calc_end()
{
   ///@todo  something
   // view->endUpdate();
}

void RtabWidget::setTableView(QTableView& tv, RModel& mm, int multiplier  )
{
    // Ширина колонок
    m_view->beginUpdate();
    for (auto cw : mm.ColumnsWidth()){
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*multiplier);
    }
    m_view->endUpdate();
}

void RtabWidget::setTableView(Qtitan::GridTableView& tv, RModel& mm, int multiplier  )
{
    tv.beginUpdate();
    m_view->tableOptions().setColumnAutoWidth(false);
    // Выравнивание
    for (auto cw : mm.ColumnsWidth())
    {
        tv.getColumn(std::get<0>(cw))->setWidth(std::get<1>(cw)*multiplier);
        tv.getColumn(std::get<0>(cw))->setTextAlignment(Qt::AlignLeft);
    }
    tv.endUpdate();
}

void RtabWidget::slot_contextMenu(ContextMenuEventArgs* args)
{
    QElapsedTimer stepTimer;
    stepTimer.start();

    args->setHandled(true);

    qInfo() << "[slot_contextMenu] Step 0 setHandled:"
            << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    const int column = args->hitInfo().columnIndex();
    const int row    = args->hitInfo().row().rowIndex();
    if (column < 0) return;

    RCol* col = m_model->getRCol(column);
    if (!col) return;

    qInfo() << "[slot_contextMenu] Step 1 (checks):"
            << stepTimer.elapsed() << "ms";
    stepTimer.restart();

    MenuContext ctx { column, row, col };
    QMenu* menu = m_menuBuilder->prepareForShow(ctx);

    qInfo() << "[slot_contextMenu] Step 2 (menu->exec):"
            << stepTimer.elapsed() << "ms";

    menu->exec(QCursor::pos());
}

void RtabWidget::slot_focusRowChanged(int /*row_old*/, int row_new)
{
    ///@todo соединение только для дочернего виджета LinkedFormController
    m_linkedFormCtrl->onParentRowChanged(row_new);
}

void RtabWidget::slot_addRow()
{
    m_view->beginUpdate();
    m_model->AddRow();
    m_view->endUpdate();
}

void RtabWidget::slot_insertRow()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->insertRows(row,1);
    m_view->endUpdate();
}

void RtabWidget::slot_duplicateRow()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->DuplicateRow(row);
    m_view->endUpdate();
}

void RtabWidget::slot_deleteRow()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->removeRows(row,1);
    m_view->endUpdate();
}

void RtabWidget::slot_beginResetModel(std::string tname)
{
    if (m_UIForm.TableName() != tname) return;
    m_view->beginUpdate();
    // Запоминаем видимость колонок
    m_columnsVisible.clear();
    for (int i = 0; i < m_view->getColumnCount(); ++i) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(m_view->getColumn(i));
        m_columnsVisible[col->caption()] = col->isVisible();
    }
}

void RtabWidget::slot_endResetModel(std::string tname)
{
    if (m_UIForm.TableName() != tname) return;
    // Восстанавливаем видимость
    for (const RCol& rcol : *m_model->getRdata()) {
        auto* col = static_cast<Qtitan::GridTableColumn*>(
            m_view->getColumn(rcol.getIndex()));
        if (!col) continue;
        col->setVisible(false);
        auto it = m_columnsVisible.find(col->caption());
        if (it != m_columnsVisible.end()){
            col->setVisible(it->second);
        }
    }
    m_view->endUpdate();
}

void RtabWidget::slot_groupCorrection()
{
    const int col = m_view->selection()->cell().columnIndex();
    RCol* prcol = m_model->getRCol(col);
    if (!prcol){
        return;
    }
    formgroupcorrection* fgc =  new formgroupcorrection(m_model->getRdata(),prcol,this);
    fgc->setAttribute(Qt::WA_DeleteOnClose);

    this->on_calc_begin();
    fgc->show();
    this->on_calc_end();
}

void RtabWidget::slot_openColProp(int col)
{
    RCol* prcol = m_model->getRCol(col);
    if (!prcol) return;
    ColPropForm* propDialog = new ColPropForm(m_model->getRdata(),
                                            m_view, prcol, this);
    propDialog->setAttribute(Qt::WA_DeleteOnClose);
    propDialog->exec();
}

void RtabWidget::slot_openSelection()
{
    // Передаём имя колонки, по которой открыто меню
    int col = m_view->selection()->cell().columnIndex();
    RCol* prcol = m_model->getRCol(col);
    std::string colName = prcol ? prcol->getColName() : "";

    FormSelection* selectionDialog = new FormSelection(m_selection,colName, this);
    connect(selectionDialog, &FormSelection::sig_selectionAccepted,
            this, &RtabWidget::slot_setFiltrForSelection);
    selectionDialog->setAttribute(Qt::WA_DeleteOnClose);
    selectionDialog->show();
}

void RtabWidget::slot_openExportCSVForm()
{
    formexportcsv* exportCsvDialog = new formexportcsv( m_model->getRdata(),this);
    exportCsvDialog->setAttribute(Qt::WA_DeleteOnClose);
    exportCsvDialog->show();
}

void RtabWidget::slot_openImportCSVForm()
{
    formimportcsv2* importCsvDialog = new formimportcsv2( m_model->getRdata(),this);
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
}

void RtabWidget::slot_condFormatsEdit(std::size_t column)
{
    m_condFormatCtrl->editCondFormats(static_cast<std::size_t>(column));
}

void RtabWidget::slot_widthByTemplate(){
    if (m_view != nullptr && m_view != nullptr){
       setTableView(*m_view,*m_model);
    }
}

void RtabWidget::slot_widthByData(){
    m_view->tableOptions().setColumnAutoWidth(true);
}

void RtabWidget::slot_setFiltrForSelection(std::string selection)
{
    // selection должен быть в формате = "pg=10"
    m_selection = selection;

    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table{ tablesx->Item(m_model->getRdata()->t_name_) };
    table->SetSelection(selection);

    DataBlock<FieldVariantData> variantBlock;
    const IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variantBlock));
    const auto indices = variantBlock.IndexesVector();

    auto* groupCondition = new GridFilterGroupCondition(m_view->filter());
    auto* condition      = new CustomFilterCondition(m_view->filter());
    groupCondition->addCondition(condition);
    for (long idx : indices)
        condition->addRow(idx);

    m_view->filter()->setCondition(groupCondition, true);
    m_view->filter()->setActive(true);
    m_view->showFilterPanel();
}

void RtabWidget::slot_itemPressed( CellClickEventArgs* _args)
{
    int row = _args->cell().rowIndex();
    int col = _args->cell().columnIndex();
    qDebug()<<"Pressed:" <<row<< ","<<col;
}

