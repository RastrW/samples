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

    view = m_grid->view< Qtitan::GridTableView>();
    view->options().setGridLines(Qtitan::LinesBoth);
    view->options().setGridLineWidth(1);
    view->tableOptions().setColumnAutoWidth(true);
    //user can select several cells at time. Hold shift key to select multiple cells.
    view->options().setSelectionPolicy(GridViewOptions::MultiCellSelection);
    view->options().setColumnHidingOnGroupingEnabled(false);
    // Sets the value that indicates whether the filter panel can automatically hide or not.
    view->options().setFilterAutoHide(true);
    // Sets the painting the doted frame around the cell with focus to the enabled. By default frame is enabled.
    view->options().setFocusFrameEnabled(true);
    // Sets the visibility status of the grid grouping panel to groupsHeader.
    view->options().setGroupsHeader(false);
    view->options().setScrollRowStyle(Qtitan::ScrollItemStyle::ScrollByItem);
    // Enables or disables wait cursor if grid is busy for lengthy operations with data like sorting or grouping.
    view->options().setShowWaitCursor(true);
    view->options().setRubberBandSelection(true);        // Выделение "резинкой"
    view->tableOptions().setColumnsHeader(true);
    view->tableOptions().setRowsQuickSelection(true);
    ///@todo Вынести в опцию контекстного меню (example MultiSelection)
    view->tableOptions().setRowFrozenButtonVisible(true);
    view->tableOptions().setFrozenPlaceQuickSelection(true);

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
    connect(view, &GridTableView::contextMenu, this, &RtabWidget::contextMenu);
    connect(view, &GridTableView::cellClicked, this, &RtabWidget::onItemPressed);

    CreateModel(pqastra,&m_UIForm);

    connect(m_pRTDM, &RTablesDataManager::sig_dataChanged,this->prm.get(),
            &RModel::slot_DataChanged);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this->prm.get(),
            &RModel::slot_BeginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this->prm.get(),
            &RModel::slot_EndResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginInsertRow,this->prm.get(),
            &RModel::slot_BeginInsertRow);
    connect(m_pRTDM, &RTablesDataManager::sig_EndInsertRow,this->prm.get(),
            &RModel::slot_EndInsertRow);

    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this,
            &RtabWidget::slot_beginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this,
            &RtabWidget::slot_endResetModel);

    m_linkedFormCtrl = std::make_unique<LinkedFormController>(
        m_pqastra,
        m_pRTDM,
        m_DockManager,
        view,
        prm.get(),
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
    int col = prm->getRdata()->mCols_.at(key);
    return std::visit(ToLong(), prm->getRdata()->pnparray_->Get(row,col));
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
    m_actAddRow = m_toolbar->addAction(QIcon(":/icons/add.png"), "Добавить");
    m_actInsertRow = m_toolbar->addAction(QIcon(":/icons/insert.png"), "Вставить");
    m_actDeleteRow = m_toolbar->addAction(QIcon(":/icons/delete.png"), "Удалить");
    m_actDuplicateRow = m_toolbar->addAction(QIcon(":/icons/duplicate.png"), "Дублировать");
    QAction* printAction = m_toolbar->addAction(QIcon(":/icons/print.png"), "Печать");

    connect(m_actAddRow,       &QAction::triggered, this, &RtabWidget::AddRow);
    connect(m_actInsertRow,    &QAction::triggered, this, &RtabWidget::insertRow_qtitan);
    connect(m_actDeleteRow,    &QAction::triggered, this, &RtabWidget::deleteRow_qtitan);
    connect(m_actDuplicateRow, &QAction::triggered, this, &RtabWidget::DuplicateRow_qtitan);
}

void RtabWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "RtabWidget::closeEvent [" << m_UIForm.Name().c_str() << "]";

    // Контроллер отключает Qt-соединения связанных форм
    m_linkedFormCtrl->disconnectAll();

    // Освобождаем DataBlock — модель перестаёт держать shared_ptr
    prm->getRdata()->pnparray_.reset();

    QWidget::closeEvent(event);
};

void RtabWidget::OnClose()
{
    this->close();
}

void RtabWidget::CreateModel(QAstra* pqastra, CUIForm* pUIForm)
{
    prm = std::make_unique<RModel>(this, pqastra, m_pRTDM);
    prm->setForm(pUIForm);
    prm->populateDataFromRastr();

    view->beginUpdate();
    view->setModel(prm.get());

    SetEditors();

    //Порядок колонок как в форме
    int vi = 0;
    for (const auto& f : pUIForm->Fields()){
        for (const RCol& rcol : *prm->getRdata()){
            if (f.Name() == rcol.getStrName()){
                column_qt = static_cast<GridTableColumn*>(
                    view->getColumn(rcol.getIndex()));
                column_qt->setVisualIndex(vi++);
                break;
            }
        }
    }

    //Show button menu for all column headers.
    //for (int i = 0; i < view->getColumnCount(); ++i)
    //    static_cast<GridTableColumn *>(view->getColumn(i))->setMenuButtonVisible(true);

    // Заливка колонок цветом по правилам CondFormat
    for (RCol& rcol : *prm->getRdata()){
        m_MapcondFormatVector.emplace(rcol.getIndex(), std::vector<CondFormat>());
    }

    std::map<int, std::vector<CondFormat>> cfv;
    CondFormatJson cfj(prm->getRdata()->t_name_ , prm->getRdata()->vCols_ ,cfv );
    cfj.from_json();
    for (auto &[key,val] : cfj.get_mcf()){
        if (m_MapcondFormatVector.find(key) != m_MapcondFormatVector.end() ){
            m_MapcondFormatVector.at(key) = val;
            prm->setCondFormats(false, key, val);
        }
    }

    view->endUpdate();

    this->update();
    this->repaint();
}

void RtabWidget::SetEditors()
{
    for (RCol& rcol : *prm->getRdata())
    {
        SetEditor(rcol);
        qDebug()<<"Col index"<<rcol.getIndex()<<"Col Name:"<<rcol.name().c_str();
    }
}

void RtabWidget::SetEditor(RCol& rcol)
{
    view->beginUpdate();
    column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.getIndex());
    if (!column_qt){
        view->endUpdate();
        return;
    }

    column_qt->setVisible(!rcol.isHidden());

    //Настройки отображения колонок по типам
    if (rcol.getComPropTT() == enComPropTT::COM_PR_ENUM)
    {
        if (!rcol.isDirectCode())
        {
            column_qt->setEditorType(GridEditor::ComboBox);

            QStringList list = prm->m_enum_.at(rcol.getIndex());

            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else
         column_qt->setEditorType(GridEditor::Numeric);
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_INT &&
        !rcol.getNameRef().empty() && contains(prm->mm_nameref_,rcol.getIndex())){
        if (!rcol.isDirectCode()){
            column_qt->setEditorType(GridEditor::ComboBox);
            //QStringList list = prm->mnamerefs_.at(rcol.index);
            QStringList list;
            for (auto val : prm->mm_nameref_.at(rcol.getIndex())){
                list.append(val.second.c_str());
            }
            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else{
            column_qt->setEditorType(GridEditor::Numeric);
        }
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_SUPERENUM &&
        !rcol.getNameRef().empty() && contains(prm->mm_superenum_,rcol.getIndex()) ){
        if (!rcol.isDirectCode()){
            column_qt->setEditorType(GridEditor::ComboBox);
            QStringList list;
            for (auto val : prm->mm_superenum_.at(rcol.getIndex())){
                list.append(val.second.c_str());
            }
            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else{
            column_qt->setEditorType(GridEditor::Numeric);
        }
    }

    if (rcol.getComPropTT() == enComPropTT::COM_PR_REAL){
        const int prec = std::atoi(rcol.prec().c_str());
        column_qt->setEditorType(GridEditor::Numeric);
        auto* repo = static_cast<GridNumericEditorRepository*>(
            column_qt->editorRepository());
        repo->setMinimum(-100000);
        repo->setMaximum( 100000);
        repo->setDecimals(prec);
    }

    if (rcol.getComPropTT() == enComPropTT::COM_PR_BOOL)
    {
        column_qt->setEditorType(GridEditor::CheckBox);
        ((Qtitan::GridCheckBoxEditorRepository *)column_qt->editorRepository())->
            setAppearance(GridCheckBox::StyledAppearance);
    }
    view->endUpdate();
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
    setTableView(*ptv,*prm);
    setTableView(*view,*prm);
}

void RtabWidget::widebydata()
{
    ptv->resizeColumnsToContents();
    view->tableOptions().setColumnAutoWidth(true);
}

void RtabWidget::setTableView(QTableView& tv, RModel& mm, int multiplier  )
{
    // Ширина колонок
    view->beginUpdate();
    for (auto cw : mm.ColumnsWidth()){
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*multiplier);
    }
    view->endUpdate();
}

void RtabWidget::setTableView(Qtitan::GridTableView& tv, RModel& mm, int multiplier  )
{
    tv.beginUpdate();
    view->tableOptions().setColumnAutoWidth(false);
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
    m_contextMenuColumn = args->hitInfo().columnIndex();
    m_contextMenuRow = args->hitInfo().row().rowIndex();
    QString qstr_col_props = "";
    RCol* prcol = nullptr;
    if (m_contextMenuColumn >= 0)
    {
        prcol = prm->getRCol(m_contextMenuColumn);
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
    view->beginUpdate();
    prm->AddRow();
    view->endUpdate();
}

void RtabWidget::insertRow_qtitan()
{
    int row = view->selection()->cell().rowIndex();

    view->beginUpdate();
    prm->insertRows(row,1);
    view->endUpdate();
}

void RtabWidget::DuplicateRow_qtitan()
{
    int row = view->selection()->cell().rowIndex();

    view->beginUpdate();
    prm->DuplicateRow(row);
    view->endUpdate();
}

void RtabWidget::deleteRow_qtitan()
{
    int row = view->selection()->cell().rowIndex();

    view->beginUpdate();
    prm->removeRows(row,1);
    view->endUpdate();
}

void RtabWidget::editCondFormats(std::size_t column)
{
    std::vector<CondFormat> condFormats;
    CondFormat condFormat;
    CondFormatManager condFormatDialog(m_MapcondFormatVector[column],
                                       "UTF-8", this);

    QString title= prm->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString();
    condFormatDialog.setWindowTitle(tr("Conditional formats for \"%1\"").
                                    arg(prm->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString()));
    if (condFormatDialog.exec()) {
        std::vector<CondFormat> condFormatVector = condFormatDialog.getCondFormats();
        prm->setCondFormats(false, column, condFormatVector);
        m_MapcondFormatVector.at(column) = condFormatVector;

        emit CondFormatsModified();
    }
}

void RtabWidget::onCondFormatsModified()
{
    CondFormatJson cfj(prm->getRdata()->t_name_ , prm->getRdata()->vCols_ ,m_MapcondFormatVector );
    cfj.save_json();
}


void RtabWidget::SetSelection(std::string Selection)
{
    m_selection = Selection;
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(prm->getRdata()->t_name_) };
    IPlainRastrResult* pres = table->SetSelection(Selection);

    DataBlock<FieldVariantData> variant_block;
    IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variant_block));
    auto vind = variant_block.IndexesVector();
    for(int ir = 0 ; ir < prm->rowCount() ; ir++)
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
    RCol* prcol = prm->getRCol(m_contextMenuColumn);
    ColPropForm* PropForm = new ColPropForm(prm->getRdata(),ptv,view, prcol);
    PropForm->show();
}

void RtabWidget::OpenSelectionForm()
{
    FormSelection* Selection = new FormSelection(this->m_selection, this);
    Selection->show();
}

void RtabWidget::OpenGroupCorrection()
{
    RCol* prcol = prm->getRCol(m_contextMenuColumn);
    formgroupcorrection* fgc =  new formgroupcorrection(prm->getRdata(),prcol,this);
    this->on_calc_begin();
    fgc->show();
    this->on_calc_end();
}

void RtabWidget::OpenExportCSVForm()
{
    formexportcsv* ExportCsv = new formexportcsv( prm->getRdata(),this);
    ExportCsv->show();
}

void RtabWidget::OpenImportCSVForm()
{
    formimportcsv2* ImportCsv = new formimportcsv2( prm->getRdata(),this);
    ImportCsv->show();
}

void RtabWidget::SetDirectCodeEntry(std::size_t column)
{
    RCol* prcol = prm->getRCol(column);
    prcol->invertDirectCodeStatus();
    SetEditor(*prcol);
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
    QModelIndexList selected = view->selection()->selectedIndexes();
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

    view->beginUpdate();

    // Запомним видимые столбцы
    int ncols = view->getColumnCount();
    for (int i = 0 ; i < ncols ; i++)
    {
        column_qt = (Qtitan::GridTableColumn *)view->getColumn(i);
        m_ColumnsVisible.insert(std::make_pair(column_qt->caption() , column_qt->isVisible() ));
    }
}

void RtabWidget::slot_endResetModel(std::string tname)
{
    if (this->m_UIForm.TableName() != tname)
        return;

    // Установим видимые столбцы
    int ncols = view->getColumnCount();
    int sz = (prm->getRdata())->size();
    qDebug()<<"onRTDM_EndResetModel"<<QString::fromStdString(tname)<<": ncols(view) = "<<ncols;
    for (const RCol& rcol : *prm->getRdata()){
        column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.getIndex());
        column_qt->setVisible(false);
        if (contains(m_ColumnsVisible, column_qt->caption())){
            column_qt->setVisible(m_ColumnsVisible.at(column_qt->caption()));
        }
    }
    view->endUpdate();
}

