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
#include "qmcr/pyhlp.h"

namespace ads{ class CDockManager; }


RtabWidget::RtabWidget(CUIForm UIForm,QWidget *parent) :
      QWidget(parent)
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
    view->options().setColumnHidingOnGroupingEnabled(false);
    view->options().setFilterAutoHide(true);                     // Sets the value that indicates whether the filter panel can automatically hide or not.
    view->options().setFocusFrameEnabled(true);                  // Sets the painting the doted frame around the cell with focus to the enabled. By default frame is enabled.
    view->options().setGroupsHeader(false);                      // Sets the visibility status of the grid grouping panel to groupsHeader.
    view->options().setScrollRowStyle(Qtitan::ScrollItemStyle::ScrollByItem);
    view->options().setShowWaitCursor(true);                    // Enables or disables wait cursor if grid is busy for lengthy operations with data like sorting or grouping.

    ///@todo Вынести в опцию контекстного меню (example MultiSelection)
    view->tableOptions().setRowFrozenButtonVisible(true);
    view->tableOptions().setFrozenPlaceQuickSelection(true);
}

RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm, RTablesDataManager* pRTDM,
                       ads::CDockManager* pDockManager, QWidget *parent)
    : RtabWidget{UIForm,parent}
{
    m_selection = "";
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

    connect(m_pRTDM, &RTablesDataManager::sig_BeginResetModel,this, &RtabWidget::slot_beginResetModel);
    connect(m_pRTDM, &RTablesDataManager::sig_EndResetModel,this,  &RtabWidget::slot_endResetModel);
}

void RtabWidget::setPyHlp(PyHlp* pPyHlp)
{
    pPyHlp_ = pPyHlp;
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

void RtabWidget::OnClose()
{
    this->close();
}

void RtabWidget::CreateModel(QAstra* pqastra, CUIForm* pUIForm)
{
    //prm = std::unique_ptr<RModel>(new RModel(nullptr, pqastra, m_pRTDM ));
    prm = std::unique_ptr<RModel>(new RModel(this, pqastra, m_pRTDM ));
    //proxyModel = new QSortFilterProxyModel(prm.get()); // used for sorting: create proxy //https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    //proxyModel->setSourceModel(prm.get());
    prm->setForm(pUIForm);
    prm->populateDataFromRastr();

    view->beginUpdate();
    view->setModel(prm.get());
    //ptv->setModel(proxyModel);

    SetEditors();

    //Порядок колонок как в форме
    int vi = 0;
    for (const auto &f : pUIForm->Fields())
    {
        for (const RCol& rcol : *prm->getRdata())
        {
            if (f.Name() == rcol.str_name_)
            {
                column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.index);
                column_qt->setVisualIndex(vi++);
                continue;
            }
        }
    }

    //Show button menu for all column headers.
    //for (int i = 0; i < view->getColumnCount(); ++i)
    //    static_cast<GridTableColumn *>(view->getColumn(i))->setMenuButtonVisible(true);

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


    //new QAbstractItemModelTester( prm.get(), QAbstractItemModelTester::FailureReportingMode::Warning, this);


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

void RtabWidget::setTableView(QTableView& tv, RModel& mm, int myltiplier  )
{
    // Ширина колонок
    view->beginUpdate();
    for (auto cw : mm.ColumnsWidth())
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*myltiplier);
    view->endUpdate();
}
void RtabWidget::setTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier  )
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

    QShortcut *sC_CTRL_I = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_I),
                                         this,nullptr,nullptr, Qt::WidgetWithChildrenShortcut);
    QShortcut *sC_CTRL_D = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_D), this);
    QShortcut *sC_CTRL_R = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_R), this);
    QShortcut *sC_CTRL_A = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_A), this);

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

    QMenu *menu_linked_macro;
    menu_linked_macro = CunstructLinkedMacroMenu( m_UIForm.Name());
    args->contextMenu()->addMenu(menu_linked_macro);

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

QMenu* RtabWidget::CunstructLinkedMacroMenu(std::string form_name)
{
    QMenu *menu=new QMenu(this);
    menu->setTitle("Макрос");

    std::vector<int> vbindvals;
    auto table_context_macro = m_pRTDM->get("macrocontext","");
    size_t ind = 0;
    for (int irow = 0; irow<table_context_macro->RowsCount();irow++)
    {
        std::string form = std::visit(ToString(),table_context_macro->Get(irow,0));
        if (form_name == form)
        {
            long form_type = std::visit(ToLong(),table_context_macro->Get(irow,6));
            long defappendix  = std::visit(ToLong(),table_context_macro->Get(irow,4));

            if (form_type == 0 && defappendix )
            {
                LinkedMacro lm;
                lm.col = std::visit(ToString(),table_context_macro->Get(irow,1));
                lm.macrofile = std::visit(ToString(),table_context_macro->Get(irow,2));
                lm.macrodesc  = std::visit(ToString(),table_context_macro->Get(irow,3));
                lm.addstr       = std::visit(ToString(),table_context_macro->Get(irow,5));
                lm.pbaseform = this;

                QAction* LinkedFormAction = new QAction(lm.macrodesc.c_str(), menu);
                menu->addAction(LinkedFormAction);
                connect(LinkedFormAction, &QAction::triggered, [this, lm] {
                    onOpenLinkedMacro(lm); });
            }
        }
    }

    return menu;
}

void RtabWidget::SetLinkedForm( LinkedForm _lf)
{
    m_lf = _lf;

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

void RtabWidget::onOpenLinkedMacro( LinkedMacro _lm)
{
    qDebug()<<"call linked macro:"<<_lm.macrofile;

    //Макросы ищем в \Data\contextmacro заменяя расчширение .vbs -> .py
    std::filesystem::path path_macrofile = QDir::currentPath().toStdString().c_str();
    path_macrofile.append("contextmacro");
    path_macrofile.append(_lm.macrofile);
    path_macrofile.replace_extension(".py");

    if( !fs::exists(path_macrofile.c_str()))
    {
        qDebug()<<"context macro not found:"<<path_macrofile;
        return;
    }

    std::ifstream file(path_macrofile);

    // Чтение файла в строку
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // В начало макроса добавим индекс строки и информацию для отладки
    std::string debug_info = fmt::format("rastr.print(f\"run contextmacro: {}[row={}]\")\n",_lm.macrofile,row).c_str();
    std::string aRow{"aRow="};
    aRow.append(std::to_string(row));
    aRow.append("\n");
    content.insert(0,aRow);
    content.insert(0,debug_info);

    // тестовый макрос: узлы -> Отметить остров
    const PyHlp::enPythonResult PythonResult{ pPyHlp_->Run( content.data() ) };

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


void RtabWidget::slot_beginResetModel(std::string tname)
{
    if (this->m_UIForm.TableName() != tname)
        return;

    view->beginUpdate();

    // Запомним видимые столбцы
    int ncols = view->getColumnCount();
    //qDebug()<<"onRTDM_BeginResetModel"<<tname<<": ncols(view) = "<<ncols;

    for (int i = 0 ; i < ncols ; i++)
    {
        column_qt = (Qtitan::GridTableColumn *)view->getColumn(i);
        m_ColumnsVisible.insert(std::make_pair(column_qt->caption() , column_qt->isVisible() ));
        //qDebug()<<i<<":"<<column_qt->caption()<<" - "<<column_qt->isVisible();
    }
}
void RtabWidget::slot_endResetModel(std::string tname)
{
    if (this->m_UIForm.TableName() != tname)
        return;

    //view->setModel(prm.get());
    //SetEditors();

    // Установим видимые столбцы
    int ncols = view->getColumnCount();
    int sz = (prm->getRdata())->size();
    qDebug()<<"onRTDM_EndResetModel"<<QString::fromStdString(tname)<<": ncols(view) = "<<ncols;
    for (const RCol& rcol : *prm->getRdata())
    {
        column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.index);
        column_qt->setVisible(false);
        if (contains(m_ColumnsVisible, column_qt->caption()))
        {
            //qDebug()<<rcol.index<<":"<<column_qt->caption()<<" - "<<column_qt->isVisible();
            column_qt->setVisible(m_ColumnsVisible.at(column_qt->caption()));
        }
    }
    view->endUpdate();
}
