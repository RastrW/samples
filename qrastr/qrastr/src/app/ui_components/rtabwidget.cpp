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
#include "customFilterCondition.h"

#include <QtitanGrid.h>
#include <utils.h>
#include <QAbstractItemModelTester>
#include <DockManager.h>
#include <QCloseEvent>

#include "formselection.h"
#include "formgroupcorrection.h"
#include "formexportcsv.h"
#include "formimportcsv2.h"

namespace ads{ class CDockManager; }

//std::map<std::string, BrowseDataTableSettings> RtabWidget::m_settings;

RtabWidget::RtabWidget(CUIForm UIForm,QWidget *parent) :
      QWidget(parent)
      //customizeFrame(this, Qt::Popup | Qt::Window)
{
    m_UIForm = UIForm;
    ptv = new RTableView(this);

    Grid::loadTranslation();
    m_grid = new Qtitan::Grid(this);
    if (m_UIForm.Vertical())
        m_grid->setViewType(Qtitan::Grid::TableViewVertical);
    else
        m_grid->setViewType(Qtitan::Grid::TableView);

    view = m_grid->view< Qtitan::GridTableView>();


    view->options().setGridLines(Qtitan::LinesBoth);
    view->options().setGridLineWidth(1);

    view->tableOptions().setColumnAutoWidth(true);
    view->options().setSelectionPolicy(GridViewOptions::MultiCellSelection);    //user can select several cells at time. Hold shift key to select multiple cells.
    //view->options().setNewRowPlace(Qtitan::AtEnd);                        // кнока добавления строки
    //view->options().setRowRemoveEnabled(false);                           // кнока DEL в контекстном меню
    //view->options().setGestureEnabled(true);
    view->options().setColumnHidingOnGroupingEnabled(false);
    //view->options().setFastScrollEffect(true);                   //  If the option is true, then a special fade effect is enabled, which on fast data scrolling shows the frame of the rows without data inside the cells only. This allows you to increase the number of frames per second when rendering and avoid lag on large data.
    view->options().setFilterAutoHide(true);                     // Sets the value that indicates whether the filter panel can automatically hide or not.
    view->options().setFocusFrameEnabled(true);                  // Sets the painting the doted frame around the cell with focus to the enabled. By default frame is enabled.
    view->options().setGroupsHeader(false);                      // Sets the visibility status of the grid grouping panel to groupsHeader.
    //view->options().setMainMenuDisabled(true);
    view->options().setScrollRowStyle(Qtitan::ScrollItemStyle::ScrollByItem);
    view->options().setShowWaitCursor(true);                    // Enables or disables wait cursor if grid is busy for lengthy operations with data like sorting or grouping.

    // TO DO: Вынести в опцию контекстного меню (example MultiSelection)
    view->tableOptions().setRowFrozenButtonVisible(true);
    view->tableOptions().setFrozenPlaceQuickSelection(true);

    //Кнопка выбор колонок слева сверху, за собой тащит целый пустой бессмысленный столбец в котором указывается стролочка активной строки
    //view->tableOptions().setColumnsQuickMenuVisible(false);
    //view->tableOptions().setColumnsQuickCustomization(false);

    //Заполнить строку поиска
   // view->find("ШАГОЛ",Qt::CaseInsensitive,true);
    //view->findClear();

}

RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm, RTablesDataManager* pRTDM,
                       ads::CDockManager* pDockManager, QWidget *parent)
    : RtabWidget{UIForm,parent}
{
    m_selection = "";
   // m_UIForm = UIForm;
    m_pqastra = pqastra;
    m_pRTDM = pRTDM;
    m_DockManager = pDockManager;

    resize(800,500);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::ApplicationModal);

    connect(this, &RtabWidget::CondFormatsModified,this, &RtabWidget::onCondFormatsModified);

    //QTitan
    //Connect Grid's context menu handler.
    connect(view, &GridTableView::contextMenu, this, &RtabWidget::contextMenu);
    connect(view, &GridTableView::cellClicked, this, &RtabWidget::onItemPressed);

    CreateModel(pqastra,&m_UIForm);

    connect(m_pRTDM, &RTablesDataManager::sig_dataChanged,this->prm.get(), &RModel::slot_DataChanged);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this->prm.get(), &RModel::slot_BeginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this->prm.get(), &RModel::slot_EndResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_BeginInsertRow,this->prm.get(), &RModel::slot_BeginInsertRow);
    connect(m_pRTDM, &RTablesDataManager::sig_EndInsertRow,this->prm.get(), &RModel::slot_EndInsertRow);
}

void RtabWidget::closeEvent(QCloseEvent *event)
{
    qDebug()<<"RtabWidget::Destructor "<< "[" <<m_UIForm.Name().c_str() << "]";
    QWidget::closeEvent(event);
    for(auto conn : m_lf.vconn)
        disconnect(conn);

    if (event->spontaneous()) {
        qDebug("The close button was clicked");
        // do event->ignore();
        // or QWidget::closeEvent(event);
    } else {
        QWidget::closeEvent(event);
    }

    this->prm->getRdata()->pnparray_.reset();
};

void RtabWidget::onvisibilityChanged(bool visible)
{
   // if (!visible)
   //     this->close();
}

void RtabWidget::OnClose()
{
    this->close();
}

void RtabWidget::CreateModel(QAstra* pqastra, CUIForm* pUIForm)
{
    prm = std::unique_ptr<RModel>(new RModel(nullptr, pqastra, m_pRTDM ));
    prm->setForm(pUIForm);
    prm->populateDataFromRastr();

    view->beginUpdate();
    view->setModel(prm.get());

    SetEditors();

    //Порядок колонок как в форме
    int vi = 0;
    for (auto f : pUIForm->Fields())
    {
        for (RCol& rcol : *prm->getRdata())
        {
            if (f.Name() == rcol.str_name_)
            {
                column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.index);

                column_qt   ->setVisualIndex(vi++);
                continue;
            }
        }
    }

    // Заливка колонок цветом по правилам CondFormat
    for (RCol& rcol : *prm->getRdata())
        m_MapcondFormatVector.emplace(rcol.index, std::vector<CondFormat>());

    std::map<int, std::vector<CondFormat>> cfv;

    CondFormatJson cfj(prm->getRdata()->t_name_ , prm->getRdata()->vCols_ ,cfv );
    cfj.from_json();
    for (auto &[key,val] : cfj.get_mcf())
        if (m_MapcondFormatVector.find(key) != m_MapcondFormatVector.end() )
        {
            m_MapcondFormatVector.at(key) = val;
            prm->setCondFormats(false, key, val);
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
        qDebug()<<"Col index"<<rcol.index<<"Col Name:"<<rcol.name().c_str();
    }
}

void RtabWidget::SetEditor(RCol& rcol)
{
    view->beginUpdate();
    column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.index);
    if (column_qt == nullptr)
        return;

    column_qt->setVisible(!rcol.hidden);

    //Настройки отображения колонок по типам
    if (rcol.com_prop_tt == enComPropTT::COM_PR_ENUM)
    {
        if (!rcol.directcode)
        {
            column_qt->setEditorType(GridEditor::ComboBox);

            QStringList list = prm->m_enum_.at(rcol.index);

            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else
         column_qt->setEditorType(GridEditor::Numeric);
    }
    if (rcol.com_prop_tt == enComPropTT::COM_PR_INT && !rcol.nameref_.empty() && contains(prm->mm_nameref_,rcol.index))
    {
        if (!rcol.directcode)
        {
            column_qt->setEditorType(GridEditor::ComboBox);
            //QStringList list = prm->mnamerefs_.at(rcol.index);
            QStringList list;
            for (auto val : prm->mm_nameref_.at(rcol.index))
                list.append(val.second.c_str());
            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else
        {
            column_qt->setEditorType(GridEditor::Numeric);
        }
    }
    if (rcol.com_prop_tt == enComPropTT::COM_PR_SUPERENUM && !rcol.nameref_.empty() && contains(prm->mm_superenum_,rcol.index) )
    {
        //rcol.directcode = true;         // DEBUG

        if (!rcol.directcode)
        {
            column_qt->setEditorType(GridEditor::ComboBox);
            QStringList list;
            for (auto val : prm->mm_superenum_.at(rcol.index))
                list.append(val.second.c_str());
            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);
        }
        else
            column_qt->setEditorType(GridEditor::Numeric);
    }

    if (rcol.com_prop_tt == enComPropTT::COM_PR_REAL)
    {
        int prec = std::atoi(rcol.prec().c_str());

        column_qt->setEditorType(GridEditor::Numeric);
        ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setMinimum(-100000);
        ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setMaximum(100000);
        ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setDecimals(prec);
    }

    if (rcol.com_prop_tt == enComPropTT::COM_PR_BOOL)
    {
        column_qt->setEditorType(GridEditor::CheckBox);
        ((Qtitan::GridCheckBoxEditorRepository *)column_qt->editorRepository())->setAppearance(GridCheckBox::StyledAppearance);
    }
    view->endUpdate();
}

void RtabWidget::on_calc_begin()
{
    // TO DO something
    //view->beginUpdate();
}
void RtabWidget::on_calc_end()
{
    // TO DO something
   // view->endUpdate();
}
void RtabWidget::onRTDM_ResetModel(std::string tname)
{
    //prm->beginResetModel();
    CreateModel(m_pqastra,&m_UIForm);
    //ptv->update();
}

void RtabWidget::SetTableView(QTableView& tv, RModel& mm, int myltiplier  )
{
    // Ширина колонок
    view->beginUpdate();
    for (auto cw : mm.ColumnsWidth())
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*myltiplier);
    view->endUpdate();
}
void RtabWidget::SetTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier  )
{
    tv.beginUpdate();
    view->tableOptions().setColumnAutoWidth(false);
    // Ширина колонок
    // Выравнивание
    for (auto cw : mm.ColumnsWidth())
    {
        tv.getColumn(std::get<0>(cw))->setWidth(std::get<1>(cw)*myltiplier);
        tv.getColumn(std::get<0>(cw))->setTextAlignment(Qt::AlignLeft);
    }
    tv.endUpdate();
}

// QTitanGrid: ContextMenu
void RtabWidget::contextMenu(ContextMenuEventArgs* args)
{
    column = args->hitInfo().columnIndex();
    row = args->hitInfo().row().rowIndex();
    QString qstr_col_props = "";
    RCol* prcol = nullptr;
    if (column >= 0)
    {
        prcol = prm->getRCol(column);
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

    QAction* insRowAct = new QAction((QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr("Вставить"),this));
    insRowAct->setShortcut(tr("Ctrl+I"));
    insRowAct->setStatusTip(tr("Вставить строку"));
    insRowAct->setShortcutContext(Qt::WidgetWithChildrenShortcut);

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

    connect(sC_CTRL_I, &QShortcut::activated, this, &RtabWidget::insertRow_qtitan);
    connect(sC_CTRL_A, &QShortcut::activated, this, &RtabWidget::AddRow);
    connect(sC_CTRL_R, &QShortcut::activated, this, &RtabWidget::DuplicateRow_qtitan);
    connect(sC_CTRL_D, &QShortcut::activated, this, &RtabWidget::deleteRow_qtitan);
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

    if ( (!prcol->nameref_.empty() && prcol->com_prop_tt == enComPropTT::COM_PR_INT) || (prcol->com_prop_tt == enComPropTT::COM_PR_SUPERENUM) )
    {
        QAction* actdirectcode=new QAction("Прямой ввод кода", this);
        actdirectcode->setCheckable(true);
        if ( !(prcol == nullptr) && prcol->directcode)
          actdirectcode->setChecked(true);
        args->contextMenu()->addAction(actdirectcode);
        connect(actdirectcode, &QAction::triggered, this, [&]() {
            emit SetDirectCodeEntry(column);
        });
    }

    QMenu *menu_linked_forms;
    menu_linked_forms = CunstructLinkedFormsMenu( m_UIForm.Name());
    args->contextMenu()->addMenu(menu_linked_forms);

    args->contextMenu()->addAction(condFormatAction);
    connect(condFormatAction, &QAction::triggered, this, [&]() {
        emit editCondFormats(column);
    });
}

QMenu* RtabWidget::CunstructLinkedFormsMenu(std::string form_name)
{
    QMenu *menu=new QMenu(this);
    menu->setTitle("Связанные формы");

    std::vector<int> vbindvals;
    auto table_context_form = m_pRTDM->get("formcontext","");

    size_t ind = 0;
    for (int irow = 0; irow<table_context_form->RowsCount();irow++)
    {
        std::string form = std::get<std::string>(table_context_form->Get(irow,0));
        if (form_name == form)
        {
            LinkedForm lf;

            lf.linkedform = std::visit(ToString(),table_context_form->Get(irow,1));
            lf.linkedname = std::visit(ToString(),table_context_form->Get(irow,2));
            lf.selection  = std::visit(ToString(),table_context_form->Get(irow,3));
            lf.bind       = std::visit(ToString(),table_context_form->Get(irow,4));
            lf.pbaseform = this;
            //lf.FillBindVals();
            for (const auto key : split( lf.bind ,','))
            {
                int col = prm->getRdata()->mCols_.at(key);
                long val = std::visit(ToLong(),prm->getRdata()->pnparray_->Get(row,col));
                lf.vbindvals.push_back(val);
            }

            QAction* LinkedFormAction = new QAction(lf.linkedname.c_str(), menu);
            menu->addAction(LinkedFormAction);
            connect(LinkedFormAction, &QAction::triggered, [this, lf] {
                onOpenLinkedForm(lf); });
        }
    }

    return menu;
}

void RtabWidget::SetLinkedForm( LinkedForm _lf)
{
    m_lf = _lf;
    // GridFilterGroupCondition* groupCondition = new GridFilterGroupCondition(view->filter());

    std::string Selection = _lf.get_selection_result();
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(prm->getRdata()->t_name_)};
    IPlainRastrResult* pres = table->SetSelection(Selection);

    DataBlock<FieldVariantData> variant_block;
    IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variant_block));
    auto vind = variant_block.IndexesVector();

    GridFilterGroupCondition* groupCondition = new GridFilterGroupCondition(view->filter());
    CustomFilterCondition* condition = new CustomFilterCondition(view->filter());

    groupCondition->addCondition(condition);
    for (long rind : vind)
        condition->addRow(rind);

    view->filter()->setCondition(groupCondition, true );
    view->filter()->setActive(true);
    view->showFilterPanel();
}


void RtabWidget::onOpenLinkedForm( LinkedForm _lf)
{
    CUIForm* pUIForm   = m_pRTDM->getForm(_lf.linkedform);
    if (pUIForm == nullptr)
        return;

    RtabWidget *prtw = new RtabWidget(m_pqastra,*pUIForm,m_pRTDM,m_DockManager,this);
    _lf.vconn.push_back(connect(view, &GridTableView::focusRowChanged, prtw, &RtabWidget::onfocusRowChanged));

    prtw->SetLinkedForm(_lf);

    auto dw = new ads::CDockWidget( stringutils::MkToUtf8(pUIForm->Name()).c_str(), this);
    dw->setWidget(prtw->m_grid);
    connect(dw, &ads::CDockWidget::closed, prtw, &RtabWidget::OnClose);

    auto area = m_DockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    prtw->show();
}

void RtabWidget::SetDirectCodeEntry(size_t column)
{
    RCol* prcol = prm->getRCol(column);
    prcol->directcode = !prcol->directcode;
    SetEditor(*prcol);
}
void RtabWidget::editCondFormats(size_t column)
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

void RtabWidget::onLinkedFormUpdate( CellClickEventArgs* _args){}

void RtabWidget::onfocusRowChanged( int _row_old,int _row_new)
{
    qDebug()<<"Linked form catch rowchanged: old row" <<_row_old<< ", row new"<<_row_new;

    m_lf.row = _row_new;
    m_lf.FillBindVals();
    SetLinkedForm(m_lf);
}

void RtabWidget::onItemPressed( CellClickEventArgs* _args)
{
    int row = _args->cell().rowIndex();
    int col = _args->cell().columnIndex();
    qDebug()<<"Pressed:" <<row<< ","<<col;
}

void RtabWidget::insertRow()
{
    prm->insertRows(index.row(),1,index);
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
void RtabWidget::deleteRow()
{
    prm->removeRows(index.row(),1,index);
}
void RtabWidget::deleteRow_qtitan()
{
    int row = view->selection()->cell().rowIndex();

    view->beginUpdate();
    prm->removeRows(row,1);
    view->endUpdate();
}
// ширина по шаблону
void RtabWidget::widebyshabl()
{
    SetTableView(*ptv,*prm);
    SetTableView(*view,*prm);
}
// ширина по контенту
void RtabWidget::widebydata()
{
    ptv->resizeColumnsToContents();
    view->tableOptions().setColumnAutoWidth(true);
}
void RtabWidget::OpenColPropForm()
{
    RCol* prcol = prm->getRCol(column);
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
    RCol* prcol = prm->getRCol(column);
    formgroupcorrection* fgc =  new formgroupcorrection(prm->getRdata(),prcol,this);
    this->on_calc_begin();
    fgc->show();
    this->on_calc_end();
}

void RtabWidget::OpenLinkedForm(std::string name,std::string selection , std::vector<int> keys)
{

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

void RtabWidget::sortAscending()
{
    proxyModel->sort(column,Qt::AscendingOrder);
}
void RtabWidget::sortDescending()
{
    proxyModel->sort(column,Qt::DescendingOrder);
}
void RtabWidget::hideColumns()
{
    RCol* prcol = prm->getRCol(column);
    prcol->hidden = true;
    ptv->setColumnHidden(prcol->index,prcol->hidden);

    column_qt->setVisible(!prcol->hidden);
}

void RtabWidget::showAllColumns()
{
    for (auto &rcol : *this->prm->getRdata())
    {
        rcol.hidden = false;
        ptv->setColumnHidden(rcol.index,rcol.hidden);
    }
     ptv->resizeColumnsToContents();
}


void RtabWidget::onUpdate(std::string _t_name)
{
    if (prm->getRdata()->t_name_ == _t_name)
    {
        this->update();
        this->repaint();
    }
}
void RtabWidget::updateFilter(size_t column, QString value)
{
    proxyModel->setFilterRegularExpression(QRegularExpression(value));
    proxyModel->setFilterKeyColumn(column);                 //The default value is 0. If the value is -1, the keys will be read from all columns
}
void RtabWidget::onFileLoad()
{
   // CreateModel(_rh);
    prm->populateDataFromRastr();
}
void RtabWidget::update_data()
{
    prm->populateDataFromRastr();
    this->update();
    this->repaint();
}
void RtabWidget::SetSelection(std::string Selection)
{
    m_selection = Selection;
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
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



void RtabWidget::test(const QModelIndexList& fromIndices)
{
    size_t sz = fromIndices.count();
    return;
}
void RtabWidget::copyMimeData(const QModelIndexList& fromIndices, QMimeData* mimeData, const bool withHeaders, const bool inSQL)
{
    mimeData->setText("test res");
    //return;
    QModelIndexList indices = fromIndices;
//return;
    // Remove all indices from hidden columns, because if we don't, we might copy data from hidden columns as well which is very
    // unintuitive; especially copying the rowid column when selecting all columns of a table is a problem because pasting the data
    // won't work as expected.
    QMutableListIterator<QModelIndex> it(indices);
    while (it.hasNext()) {
        if ( this->ptv->isColumnHidden(it.next().column()))
            it.remove();
    }

    // Abort if there's nothing to copy
    if (indices.isEmpty())
        return;

    //RModel* m = qobject_cast<RModel*>( this->ptv->model());
    RModel* m =  this->prm.get();


    // Clear internal copy-paste buffer
    m_buffer.clear();

    // If a single cell is selected which contains an image, copy it to the clipboard
    if (!inSQL && !withHeaders && indices.size() == 1) {
        QImage img;
        QVariant varData = m->data(indices.first(), Qt::EditRole);

        if (img.loadFromData(varData.toByteArray()))
        {
            // If it's an image, copy the image data to the clipboard
            mimeData->setImageData(img);
            return;
        }
    }

    // If we got here, a non-image cell was or multiple cells were selected, or copy with headers was requested.
    // In this case, we copy selected data into internal copy-paste buffer and then
    // we write a table both in HTML and text formats to the system clipboard.

    // Copy selected data into internal copy-paste buffer
    int last_row = indices.first().row();
    BufferRow lst;
    for(int i=0;i<indices.size();i++)
    {
        if(indices.at(i).row() != last_row)
        {
            m_buffer.push_back(lst);
            lst.clear();
        }
        lst.push_back(indices.at(i).data(Qt::EditRole).toByteArray());
        last_row = indices.at(i).row();
    }
    m_buffer.push_back(lst);

    // TSV text or SQL
    QString result;

    // HTML text
    QString htmlResult = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
                         "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
                         "<title></title>";

    // The generator-stamp is later used to know whether the data in the system clipboard is still ours.
    // In that case we will give precedence to our internal copy buffer.
    QString now = QDateTime::currentDateTime().toString("YYYY-MM-DDTHH:mm:ss.zzz");
    m_generatorStamp = QString("<meta name=\"generator\" content=\"%1\"><meta name=\"date\" content=\"%2\">").arg(QApplication::applicationName().toHtmlEscaped(), now);
    htmlResult.append(m_generatorStamp);
    // TODO: is this really needed by Excel, since we use <pre> for multi-line cells?
    htmlResult.append("<style type=\"text/css\">br{mso-data-placement:same-cell;}</style></head><body>"
                      "<table border=1 cellspacing=0 cellpadding=2>");

    // Insert the columns in a set, since they could be non-contiguous.
    std::set<int> colsInIndexes, rowsInIndexes;
    //for(const QModelIndex & idx : qAsConst(indices)) {
    for(const QModelIndex & idx : std::as_const(indices)) {
        colsInIndexes.insert(idx.column());
        rowsInIndexes.insert(idx.row());
    }

    const QString fieldSepText = "\t";
#ifdef Q_OS_WIN
    const QString rowSepText = "\r\n";
#else
    const QString rowSepText = "\n";
#endif


    int firstColumn = *colsInIndexes.begin();

    QString sqlInsertStatement;
    // Table headers
    if (withHeaders || inSQL) {
        if (inSQL)
            sqlInsertStatement = QString("INSERT INTO %1 (").arg(QString::fromStdString(m->getRdata()->t_name_));
        else
            htmlResult.append("<tr><th>");

        for(int col : colsInIndexes) {
            QByteArray headerText_ = ptv->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toByteArray();
            std::string headerText = ptv->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString().toUtf8().constData();
            if (col != firstColumn) {
                if (inSQL)
                    sqlInsertStatement.append(", ");
                else {
                    result.append(fieldSepText);
                    htmlResult.append("</th><th>");
                }
            }
            if (inSQL)
            {
                //sqlInsertStatement.append(sqlb::escapeIdentifier(headerText));
            }
            else {
                result.append(headerText.c_str());
                htmlResult.append(headerText.c_str());
            }
        }
        if (inSQL)
            sqlInsertStatement.append(") VALUES (");
        else {
            result.append(rowSepText);
            htmlResult.append("</th></tr>");
        }
    }


 /*   QProgressDialog progress(this);
    progress.setWindowModality(Qt::ApplicationModal);
    // Disable context help button on Windows
    progress.setWindowFlags(progress.windowFlags()
                            & ~Qt::WindowContextHelpButtonHint);
    progress.setRange(*rowsInIndexes.begin(), *rowsInIndexes.end());
    progress.setMinimumDuration(2000);
*/

    // Iterate over rows x cols checking if the index actually exists when needed, in order
    // to support non-rectangular selections.



    for(const int row : rowsInIndexes) {

        // Beginning of row
        if (inSQL)
        {
            //result.append(sqlInsertStatement);
        }
        else
            htmlResult.append("<tr>");

        for(const int column : colsInIndexes) {

            const QModelIndex index = indices.first().sibling(row, column);
            const bool isContained = indices.contains(index);

            if (column != firstColumn) {
                // Add text separators
                if (inSQL)
                    result.append(", ");
                else
                    result.append(fieldSepText);
            }

            if(isContained) {
                /*
                QFont font;
                font.fromString(index.data(Qt::FontRole).toString());

                const Qt::Alignment align(index.data(Qt::TextAlignmentRole).toInt());
                const QString textAlign(CondFormat::alignmentTexts().at(CondFormat::fromCombinedAlignment(align)).toLower());
                htmlResult.append(QString("<td style=\"font-family:'%1';font-size:%2pt;font-style:%3;font-weight: %4;%5 "
                                          "background-color:%6;color:%7;text-align:%8\">").arg(
                                          font.family().toHtmlEscaped(), // font-family
                                          QString::number(font.pointSize()), // font-size
                                          font.italic() ? "italic" : "normal", // font-style,
                                          font.bold() ? "bold" : "normal", // font-weigth,
                                          font.underline() ? " text-decoration: underline;" : "", // text-decoration,
                                          index.data(Qt::BackgroundRole).toString(), // background-color
                                          index.data(Qt::ForegroundRole).toString(), // color
                                          textAlign));
                    */
            } else {
                htmlResult.append("<td>");
            }
            QImage img;
            const QVariant bArrdata = isContained ? index.data(Qt::EditRole) : QVariant();

            if (bArrdata.isNull()) {
                // NULL data: NULL in SQL, empty in HTML or text.
                if (inSQL) result.append("NULL");
            } else if(!m->isBinary(index)) {
                // Text data
                QByteArray text = bArrdata.toByteArray();

                if (inSQL)
                {
                    //result.append(sqlb::escapeString(text));
                }
                else {
                    result.append(text);
                    // Table cell data: text
                    if (text.contains('\n') || text.contains('\t'))
                        htmlResult.append(QString("<pre>%1</pre>").arg(QString(text).toHtmlEscaped()));
                    else
                        htmlResult.append(QString(text).toHtmlEscaped());
                }
            } else if (inSQL) {
                // Table cell data: binary in SQL. Save as BLOB literal.
                result.append(QString("X'%1'").arg(QString(bArrdata.toByteArray().toHex())));
            } else if (img.loadFromData(bArrdata.toByteArray())) {
                // Table cell data: image. Store it as an embedded image in HTML
                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                img.save(&buffer, "PNG");
                buffer.close();

                htmlResult.append(QString("<img src=\"data:image/png;base64,%1\" alt=\"Image\">")
                                      .arg(QString(ba.toBase64())));
                result.append(index.data(Qt::DisplayRole).toByteArray());
            }
            else {
                //result.append(QString("%1").arg(QString(bArrdata.toByteArray().toHex())));
                //result.append(QString("%1").arg(QString(bArrdata.toString())));

                QByteArray text = bArrdata.toByteArray();
                result.append(text);
                // Table cell data: text
                if (text.contains('\n') || text.contains('\t'))
                    htmlResult.append(QString("<pre>%1</pre>").arg(QString(text).toHtmlEscaped()));
                else
                    htmlResult.append(QString(text).toHtmlEscaped());
            }


            // End of column
            // Add HTML cell terminator
            htmlResult.append("</td>");
        }

        // End of row
        if (inSQL)
            result.append(");");
        else
            htmlResult.append("</tr>");
        result.append(rowSepText);

        /*
        progress.setValue(row);
        // Abort the operation if the user pressed ESC key or Cancel button
        if (progress.wasCanceled()) {
            return;
        }
        */
    }

    if (!inSQL) {
        htmlResult.append("</table></body></html>");
        mimeData->setHtml(htmlResult);
    }
#if(defined(_MSC_VER))
    result.removeLast();
#else
    result.chop(1);
#endif
    QString test = "test str\r\n";
    QString test2 = "test str\r\n";
#if(defined(_MSC_VER))
    test.removeLast();
#else
    test.chop(1);
#endif
    test2.remove(test2.length()-2,2);
    //result.remove(result.length()-2,2);

    int pos = result.lastIndexOf(rowSepText);
    result = result.left(pos);
    mimeData->setText(result.toUtf8().data());

}

void RtabWidget::copy(const bool withHeaders, const bool inSQL )
{
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(tr("test"));

    copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);  // так падает https://stackoverflow.com/questions/15123109/crash-with-qitemselectionmodelselectedindexes
    // TO DO : выяснить почему падает !
    //test(this->ptv->selectionModel()->selectedIndexes().toList());
    // Падает только в DEBUG так как QT  использует библиотеки от Release и в этом случае деструктор отрабатывает некорректно.
    // В  Release не падает все ОК.

    qApp->clipboard()->setMimeData(mimeData);
}
void RtabWidget::copy()
{
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(tr("test"));
    //copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);
    //copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);
    test(this->ptv->selectionModel()->selectedIndexes());
    qApp->clipboard()->setMimeData(mimeData);

}
std::tuple<int,double> RtabWidget::GetSumSelected()
{
    // QModelIndexList selected = this->ptv->selectionModel()->selectedIndexes();
    QModelIndexList selected = view->selection()->selectedIndexes();
    if (selected.empty())
        return (std::make_tuple(0,0));

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
    return std::make_tuple(number,total);
}




