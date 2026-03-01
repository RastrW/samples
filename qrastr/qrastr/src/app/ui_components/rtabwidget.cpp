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
#include "CondFormat.h"
#include "qastra.h"
#include "delegatecombobox.h"
#include <QShortcut>
#include <QPalette>
#include "CondFormat.h"
#include "CondFormatManager.h"
#include "condformatjson.h"
#include "linkedform.h"

#include <QtitanGrid.h>
#include <utils.h>
#include <QAbstractItemModelTester>
#include <DockManager.h>
#include <QCloseEvent>


#include "formselection.h"
#include "formgroupcorrection.h"
#include "formexportcsv.h"
#include "formimportcsv2.h"
#include "qmcr/pyhlp.h"
#include <QToolBar>
#include "linkedformcontroller.h"
#include "rtableview.h"
#include "ColPropForm.h"

RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm, RTablesDataManager* pRTDM,
                       ads::CDockManager* pDockManager, QWidget *parent)
    : QWidget(parent),
    m_UIForm {UIForm},
    m_pqastra {pqastra},
    m_pRTDM {pRTDM},
    m_DockManager {pDockManager}
{
    ptv = new RTableView(this);

    //  Настройка QTitan Grid
    Grid::loadTranslation();
    m_grid = new Qtitan::Grid(this);
    if (m_UIForm.Vertical()){
        m_grid->setViewType(Qtitan::Grid::TableViewVertical);
    }
    else{
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

    //  Горячие клавиши
    QShortcut *sC_CTRL_I = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_I),
                                         this,nullptr,nullptr, Qt::WidgetWithChildrenShortcut);
    QShortcut *sC_CTRL_D = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_D), this);
    QShortcut *sC_CTRL_R = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_R), this);
    QShortcut *sC_CTRL_A = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_A), this);

    connect(sC_CTRL_I, &QShortcut::activated, this, &RtabWidget::insertRow_qtitan);
    connect(sC_CTRL_A, &QShortcut::activated, this, &RtabWidget::AddRow);
    connect(sC_CTRL_R, &QShortcut::activated, this, &RtabWidget::DuplicateRow_qtitan);
    connect(sC_CTRL_D, &QShortcut::activated, this, &RtabWidget::deleteRow_qtitan);

    resize(800,500);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::ApplicationModal);
    //  Grid-соединения
    connect(this, &RtabWidget::CondFormatsModified,this, &RtabWidget::onCondFormatsModified);

    //QTitan
    //Connect Grid's context menu handler.
    connect(m_view, &GridTableView::contextMenu, this, &RtabWidget::contextMenu);
    connect(m_view, &GridTableView::cellClicked, this, &RtabWidget::onItemPressed);

    CreateModel(pqastra,&m_UIForm);

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

    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this,
            &RtabWidget::slot_beginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this,
            &RtabWidget::slot_endResetModel);

    m_linkedFormCtrl = std::make_unique<LinkedFormController>(
        m_pqastra,
        m_pRTDM,
        m_DockManager,
        m_view,
        m_model.get(),
        m_UIForm,
        this);
}

QWidget* RtabWidget::createDockContent(bool addToolbar) {
    QWidget* wrapper = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(wrapper);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    if (addToolbar){
        setupToolbar();
        layout->addWidget(m_toolbar);
        layout->addWidget(m_grid);
    }else{
        layout->addWidget(m_grid);
    }

    return wrapper;
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

    connect(m_actAddRow,       &QAction::triggered, this, &RtabWidget::AddRow);
    connect(m_actInsertRow,    &QAction::triggered, this, &RtabWidget::insertRow_qtitan);
    connect(m_actDeleteRow,    &QAction::triggered, this, &RtabWidget::deleteRow_qtitan);
    connect(m_actDuplicateRow, &QAction::triggered, this, &RtabWidget::DuplicateRow_qtitan);
    connect(m_groupCorrection, &QAction::triggered, this, &RtabWidget::OpenGroupCorrection);
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

void RtabWidget::OnClose()
{
    this->close();
}

void RtabWidget::CreateModel(QAstra* pqastra, CUIForm* pUIForm)
{
    m_model = std::make_unique<RModel>(this, pqastra, m_pRTDM);
    m_model->setForm(pUIForm);
    m_model->populateDataFromRastr();

    m_view->beginUpdate();
    m_view->setModel(m_model.get());

    applyAllColumnEditors ();

    //Порядок колонок как в форме
    int vi = 0;
    for (const auto& f : pUIForm->Fields()){
        for (const RCol& rcol : *m_model->getRdata()){
            if (f.Name() == rcol.getStrName()){
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

    // Заливка колонок цветом по правилам CondFormat
    for (RCol& rcol : *m_model->getRdata()){
        m_MapcondFormatVector.emplace(rcol.getIndex(), std::vector<CondFormat>());
    }

    std::map<int, std::vector<CondFormat>> cfv;
    CondFormatJson cfj(m_model->getRdata()->t_name_ , m_model->getRdata()->vCols_ ,cfv );
    cfj.from_json();
    for (auto &[key,val] : cfj.get_mcf()){
        if (m_MapcondFormatVector.find(key) != m_MapcondFormatVector.end() ){
            m_MapcondFormatVector.at(key) = val;
            m_model->setCondFormats(false, key, val);
        }
    }

    m_view->endUpdate();

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

void RtabWidget::widebyshabl()
{
    setTableView(*ptv,*m_model);
    setTableView(*m_view,*m_model);
}

void RtabWidget::widebydata()
{
    ptv->resizeColumnsToContents();
    m_view->tableOptions().setColumnAutoWidth(true);
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

void RtabWidget::contextMenu(ContextMenuEventArgs* args)
{
    QMenu* menu = args->contextMenu();

    // Перебираем встроенные экшены и удаляем ненужные
    const QList<QAction*> actions = menu->actions();
    for (QAction* act : actions)
    {
        const QString text = act->text();
        // Убираем "Подогнать" (Fit) и "Удалить строку" (Delete row)
        if (text.contains("Подогнать", Qt::CaseInsensitive) ||
            text.contains("Удалить",   Qt::CaseInsensitive))
        {
            menu->removeAction(act);
            act->setShortcut(QKeySequence());
            act->deleteLater(); // если нужно освободить память
        }
    }

    m_contextMenuColumn = args->hitInfo().columnIndex();
    m_contextMenuRow = args->hitInfo().row().rowIndex();
    QString qstr_col_props = "";
    RCol* prcol = nullptr;
    if (m_contextMenuColumn >= 0)
    {
        prcol = m_model->getRCol(m_contextMenuColumn);
        std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| -(" + prcol->name() + "), [" +prcol->unit() + "]";
        qstr_col_props = str_col_prop.c_str();
    }
    if (!prcol)
        return;

    QAction* condFormatAction = new QAction(QIcon(":/icons/edit_cond_formats"),
                                            tr("Условное форматирование"),  args->contextMenu());

    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(
        qstr_col_props,
        this,
        &RtabWidget::OpenColPropForm);


    std::tuple<int,double> item_sum = GetSumSelected();
#if(defined(_MSC_VER))
    args->contextMenu()->addAction("Сумма: " + QString::number(std::get<1>(item_sum))+" Элементов: " + QString::number(std::get<0>(item_sum)),this,nullptr);
#else
    args->contextMenu()->addAction( QString("Сумма: ") + QString::number(std::get<1>(item_sum))+QString(" Элементов: ") + QString::number(std::get<0>(item_sum)) );
#endif

    args->contextMenu()->addSeparator();

    ///@todo
    // В Qtitane не работают шорткаты, хотя del встроенный работает.
    args->contextMenu()->addAction(
                           QIcon(":/images/Rastr3_grid_insrow_16x16.png"),
                           tr("Вставить"),
                           this,
                           &RtabWidget::insertRow_qtitan
                           )->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    args->contextMenu()->addAction(
                           QIcon(":/images/Rastr3_grid_addrow_16x16.png"),
                           tr("Добавить"),
                           this,
                           &RtabWidget::AddRow
                           )->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    args->contextMenu()->addAction(
                           QIcon(":/images/Rastr3_grid_duprow_16x161.png"),
                           tr("Дублировать"),
                           this,
                           &RtabWidget::DuplicateRow_qtitan
                           )->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    args->contextMenu()->addAction(
                           QIcon(":/images/Rastr3_grid_delrow_16x16.png"),
                           tr("Удалить"),
                           this,
                           &RtabWidget::deleteRow_qtitan
                           )->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    args->contextMenu()->addAction(
                           QIcon(":/images/column_edit.png"),
                           tr("Групповая коррекция"),
                           this,
                           &RtabWidget::OpenGroupCorrection);

    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(tr("Выравнивание: по шаблону"),
                                   this, &RtabWidget::widebyshabl);
    args->contextMenu()->addAction(tr("Выравнивание: по данным"),
                                   this, &RtabWidget::widebydata);
    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(tr("Экспорт CSV"),
                                   this, &RtabWidget::OpenExportCSVForm);
    args->contextMenu()->addAction(tr("Импорт CSV"),
                                   this, &RtabWidget::OpenImportCSVForm);
    args->contextMenu()->addAction(tr("Выборка"),
                                   this, &RtabWidget::OpenSelectionForm);

    if ((!prcol->getNameRef().empty() && prcol->getComPropTT() == enComPropTT::COM_PR_INT)
        || (prcol->getComPropTT() == enComPropTT::COM_PR_SUPERENUM))
    {
        QAction* actdirectcode=new QAction("Прямой ввод кода", this);
        actdirectcode->setCheckable(true);
        if ( !(prcol == nullptr) && prcol->isDirectCode())
          actdirectcode->setChecked(true);
        args->contextMenu()->addAction(actdirectcode);
        connect(actdirectcode, &QAction::triggered, this, [&]() {
            emit SetDirectCodeEntry(m_contextMenuColumn);
        });
    }

    //  Подменю «Связанные формы» и «Макрос»
    args->contextMenu()->addMenu(
        m_linkedFormCtrl->buildLinkedFormsMenu(m_contextMenuRow));
    args->contextMenu()->addMenu(
        m_linkedFormCtrl->buildLinkedMacroMenu(m_contextMenuRow));

    args->contextMenu()->addAction(condFormatAction);
    connect(condFormatAction, &QAction::triggered, this, [&]() {
        emit editCondFormats(m_contextMenuColumn);
    });
}

void RtabWidget::onfocusRowChanged(int /*row_old*/, int row_new)
{
    qDebug() << "RtabWidget::onfocusRowChanged: row =" << row_new;
    m_linkedFormCtrl->onParentRowChanged(row_new);
}

void RtabWidget::AddRow()
{
    m_view->beginUpdate();
    m_model->AddRow();
    m_view->endUpdate();
}

void RtabWidget::insertRow_qtitan()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->insertRows(row,1);
    m_view->endUpdate();
}

void RtabWidget::DuplicateRow_qtitan()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->DuplicateRow(row);
    m_view->endUpdate();
}

void RtabWidget::deleteRow_qtitan()
{
    int row = m_view->selection()->cell().rowIndex();

    m_view->beginUpdate();
    m_model->removeRows(row,1);
    m_view->endUpdate();
}

void RtabWidget::editCondFormats(std::size_t column)
{
    std::vector<CondFormat> condFormats;
    CondFormat condFormat;
    CondFormatManager condFormatDialog(m_MapcondFormatVector[column],
                                       "UTF-8", this);

    QString title= m_model->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString();
    condFormatDialog.setWindowTitle(tr("Conditional formats for \"%1\"").
                                    arg(m_model->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString()));
    if (condFormatDialog.exec()) {
        std::vector<CondFormat> condFormatVector = condFormatDialog.getCondFormats();
        m_model->setCondFormats(false, column, condFormatVector);
        m_MapcondFormatVector.at(column) = condFormatVector;

        emit CondFormatsModified();
    }
}

void RtabWidget::onCondFormatsModified()
{
    CondFormatJson cfj(m_model->getRdata()->t_name_,
                       m_model->getRdata()->vCols_ ,m_MapcondFormatVector );
    cfj.save_json();
}


void RtabWidget::SetSelection(std::string Selection)
{
    m_selection = Selection;
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_model->getRdata()->t_name_) };
    IPlainRastrResult* pres = table->SetSelection(Selection);

    DataBlock<FieldVariantData> variant_block;
    IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variant_block));
    auto vind = variant_block.IndexesVector();
    for(int ir = 0 ; ir < m_model->rowCount() ; ir++)
    {
        ptv->setRowHidden(ir,true);
        // TO DO: add hide rows for view
    }

    for (IndexT &ind : vind)
    {
        qDebug() << ind;
        ptv->setRowHidden(ind,false);
    }
}

void RtabWidget::OpenColPropForm()
{
    RCol* prcol = m_model->getRCol(m_contextMenuColumn);
    ColPropForm* PropForm = new ColPropForm(m_model->getRdata(),ptv,m_view, prcol);
    PropForm->show();
}

void RtabWidget::OpenSelectionForm()
{
    FormSelection* Selection = new FormSelection(this->m_selection, this);
    Selection->show();
}

void RtabWidget::OpenGroupCorrection()
{
    RCol* prcol = m_model->getRCol(m_contextMenuColumn);
    formgroupcorrection* fgc =  new formgroupcorrection(m_model->getRdata(),prcol,this);
    this->on_calc_begin();
    fgc->show();
    this->on_calc_end();
}

void RtabWidget::OpenExportCSVForm()
{
    formexportcsv* ExportCsv = new formexportcsv( m_model->getRdata(),this);
    ExportCsv->show();
}

void RtabWidget::OpenImportCSVForm()
{
    formimportcsv2* ImportCsv = new formimportcsv2( m_model->getRdata(),this);
    ImportCsv->show();
}

void RtabWidget::SetDirectCodeEntry(std::size_t column)
{
    RCol* prcol = m_model->getRCol(column);
    prcol->invertDirectCodeStatus();
    applyColumnEditor(column);
}

void RtabWidget::onItemPressed( CellClickEventArgs* _args)
{
    int row = _args->cell().rowIndex();
    int col = _args->cell().columnIndex();
    qDebug()<<"Pressed:" <<row<< ","<<col;
}

std::tuple<int,double>
RtabWidget::GetSumSelected()
{
    QModelIndexList selected = m_view->selection()->selectedIndexes();
    if (selected.empty()){
        return {0, 0.0};
    }

    int number = 0;
    double total = 0;

    for (QModelIndex item : selected) {
        bool ok;
        double value = item.data().toDouble(&ok);

        if (ok) {
            total += value;
            number++;
        }
    }
    return {number,total};
}

void RtabWidget::slot_beginResetModel(std::string tname)
{
    if (this->m_UIForm.TableName() != tname)
        return;

    m_view->beginUpdate();

    // Запомним видимые столбцы
    int ncols = m_view->getColumnCount();
    for (int i = 0 ; i < ncols ; i++)
    {
        column_qt = (Qtitan::GridTableColumn *)m_view->getColumn(i);
        m_ColumnsVisible.insert(std::make_pair(column_qt->caption() , column_qt->isVisible() ));
    }
}

void RtabWidget::slot_endResetModel(std::string tname)
{
    if (this->m_UIForm.TableName() != tname)
        return;

    // Установим видимые столбцы
    int ncols = m_view->getColumnCount();
    int sz = (m_model->getRdata())->size();
    qDebug()<<"onRTDM_EndResetModel"<<QString::fromStdString(tname)<<": ncols(view) = "<<ncols;
    for (const RCol& rcol : *m_model->getRdata()){
        column_qt = (Qtitan::GridTableColumn *)m_view->getColumn(rcol.getIndex());
        column_qt->setVisible(false);
        if (contains(m_ColumnsVisible, column_qt->caption())){
            column_qt->setVisible(m_ColumnsVisible.at(column_qt->caption()));
        }
    }
    m_view->endUpdate();
}

