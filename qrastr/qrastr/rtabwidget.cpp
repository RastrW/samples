#include "rtabwidget.h"
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QLineEdit>
#include <QScrollBar>
#include "FilterTableHeader.h"
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QDateTime>
#include <QProgressDialog>
#include "CondFormat.h"
#include "qastra.h"
//using WrapperExceptionType = std::runtime_error;
//#include "IPlainRastrWrappers.h"
#include "delegatecombobox.h"
#include "delegatedoubleitem.h"
#include "delegatecheckbox.h"
#include <QShortcut>
#include <QPalette>
#include "CondFormat.h"
#include "CondFormatManager.h"
#include "condformatjson.h"
#include "linkedform.h"

#include "Settings.h"
#include <QtitanGrid.h>
#include <utils.h>
#include "License2/json.hpp"

#include <DockManager.h>
#include <QCloseEvent>

namespace ads{ class CDockManager; }

//std::map<std::string, BrowseDataTableSettings> RtabWidget::m_settings;

RtabWidget::RtabWidget(QWidget *parent) :
      QWidget(parent),
      customizeFrame(this, Qt::Popup | Qt::Window)
{
    ptv = new RTableView(this);

    Grid::loadTranslation();
    m_grid = new Qtitan::Grid();
    m_grid->setViewType(Qtitan::Grid::TableView);
    view = m_grid->view< Qtitan::GridTableView>();

    view->options().setGridLines(Qtitan::LinesBoth);
    view->options().setGridLineWidth(1);
    view->tableOptions().setColumnAutoWidth(true);
    view->options().setSelectionPolicy(GridViewOptions::MultiCellSelection);
     //view->options().f
    //view->tableOptions().fi
    //view->options().setGestureEnabled(true);

    // TO DO: Вынести в опцию контекстного меню (example MultiSelection)
    view->tableOptions().setRowFrozenButtonVisible(true);
    view->tableOptions().setFrozenPlaceQuickSelection(true);

    //Кнопка выбор колонок слева сверху, за собой тащит целый пустой бессмысленный столбец в котором указывается стролочка активной строки
    //view->tableOptions().setColumnsQuickMenuVisible(false);
    //view->tableOptions().setColumnsQuickCustomization(false);

    //view->tableOptions().set

    //Заполнить строку поиска
   // view->find("ШАГОЛ",Qt::CaseInsensitive,true);
    //view->findClear();

}

RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm,RTablesDataManager* pRTDM, ads::CDockManager* pDockManager, QWidget *parent)
    : RtabWidget{parent}
{
    m_selection = "";
    m_UIForm = UIForm;
    m_pqastra = pqastra;
    m_pRTDM = pRTDM;
    m_DockManager = pDockManager;

    customizeFrame.setObjectName("RTABLEVIEWCUSTOMIZEFRAME");
    customizeFrame.setFrameShape(QFrame::StyledPanel);
    customizeFrame.raise();
    customizeFrame.setMinimumHeight(200);
    customizeFrame.setMinimumWidth(175);
    customizeFrame.resize(175, 100);

    customizeListWidget.setParent(&customizeFrame);
    customizeListWidget.setObjectName("RTABLEVIEWCUSTOMIZEFRAMELISTWIDGET");
    customizeListWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customizeListWidget.resize(175, 200);
    customizeListWidget.setResizeMode(QListView::Adjust);

    customizeFrame.hide();

    resize(800,500);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::ApplicationModal);


    //connect(this, SIGNAL(CondFormatsModified),this, SLOT(onCondFormatsModified()));
    connect(this, &RtabWidget::CondFormatsModified,this, &RtabWidget::onCondFormatsModified);
    connect(m_pRTDM, SIGNAL(RTDM_UpdateModel(std::string)), this, SLOT(onRTDM_UpdateModel(std::string)));
    connect(m_pRTDM, SIGNAL(RTDM_UpdateView(std::string)), this, SLOT(onRTDM_UpdateView(std::string)));

    connect(ptv, SIGNAL(onCornerButtonPressed()), SLOT(cornerButtonPressed()));

    connect(this->ptv, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customMenuRequested(QPoint)));

    connect(this->ptv, SIGNAL(ContextMenu(QPoint)),
            SLOT(customMenuRequested(QPoint)));

    connect(this->ptv, SIGNAL(pressed(const QModelIndex &)),
            SLOT(onItemPressed(const QModelIndex &)));
    connect(this->ptv->horizontalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customHeaderMenuRequested(QPoint)));
    connect(ptv->horizontalHeader(), SIGNAL(filterChanged(size_t , QString )), this, SLOT(updateFilter(size_t , QString) ));

    //QTitan
    //Connect Grid's context menu handler.
    connect(view, SIGNAL(contextMenu(ContextMenuEventArgs*)), this, SLOT(contextMenu(ContextMenuEventArgs* )));

    connect(this->view, SIGNAL(cellClicked( CellClickEventArgs* )), this,
            SLOT(onItemPressed( CellClickEventArgs*)));


    CreateModel(pqastra,&m_UIForm);

    //SetTableView(*ptv,*prm);                // ширина по шаблону
    ptv->resizeColumnsToContents();         // ширина по контенту
    ptv->setParent(this);

    int ncols = prm->columnCount();
    ptv->generateFilters(ncols);
    ptv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ptv);
    setLayout(layout);
}


void RtabWidget::closeEvent(QCloseEvent *event)
{
    qDebug()<<"RtabWidget::Destructor "<< "[" <<m_UIForm.Name().c_str() << "]";
    QWidget::closeEvent(event);
    disconnect(m_lf.conn);
    if (event->spontaneous()) {
        qDebug("The close button was clicked");
        // do event->ignore();
        // or QWidget::closeEvent(event);
    } else {
        QWidget::closeEvent(event);
    }
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
    proxyModel = new QSortFilterProxyModel(prm.get()); // used for sorting: create proxy //https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    proxyModel->setSourceModel(prm.get());
    prm->setForm(pUIForm);
    prm->populateDataFromRastr();

    view->beginUpdate();
    view->setModel(prm.get());
    ptv->setModel(proxyModel);

    for (RCol& rcol : *prm->getRdata())
    {
        column_qt = (Qtitan::GridTableColumn *)view->getColumn(rcol.index);

        //Видимость колонок
        ptv->setColumnHidden(rcol.index,rcol.hidden);
        column_qt->setVisible(!rcol.hidden);

        //Настройки отображения колонок по типам
        if (rcol.com_prop_tt == enComPropTT::COM_PR_ENUM)
        {
            DelegateComboBox* delegate = new DelegateComboBox(this,rcol.NameRef());
            ptv->setItemDelegateForColumn(rcol.index, delegate);
            column_qt->setEditorType(GridEditor::ComboBox);

            QStringList list = prm->mnamerefs_.at(rcol.index);
            column_qt->editorRepository()->setDefaultValue(list.at(0), Qt::EditRole);
            column_qt->editorRepository()->setDefaultValue(list, (Qt::ItemDataRole)Qtitan::ComboBoxRole);

            // Make the combo boxes always displayed.
            /*for ( int i = 0; i < prm->rowCount(); ++i )
            {
                ptv->openPersistentEditor( prm->index(i, rcol.index) );
            }*/
        }

        if (rcol.com_prop_tt == enComPropTT::COM_PR_REAL)
        {
            int prec = std::atoi(rcol.prec().c_str());
            DelegateDoubleItem* delegate = new DelegateDoubleItem(prec,this);
            ptv->setItemDelegateForColumn(rcol.index, delegate);

            column_qt->setEditorType(GridEditor::Numeric);
            ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setMinimum(-100000);
            ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setMaximum(100000);
            ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setDecimals(prec);
        }

        if (rcol.com_prop_tt == enComPropTT::COM_PR_BOOL)
        {
            DelegateCheckBox* delegate = new DelegateCheckBox(this);
            ptv->setItemDelegateForColumn(rcol.index, delegate);

            column_qt->setEditorType(GridEditor::CheckBox);
            ((Qtitan::GridCheckBoxEditorRepository *)column_qt->editorRepository())->setAppearance(GridCheckBox::StyledAppearance);
        }
    }

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
    this->update();
    this->repaint();
}

void RtabWidget::onRTDM_UpdateModel(std::string tname)
{
    CreateModel(m_pqastra,&m_UIForm);
    ptv->update();
}
void RtabWidget::onRTDM_UpdateView(std::string tname)
{
    // Не работает, при добавлении строки вторая таблица не обновляется пока не ткнешь в нее мышь
    ptv->update();
    ptv->repaint();
    QPalette paletteBGColor;
    QBrush brush;
    brush.setColor(Qt::black);
    paletteBGColor.setBrush(QPalette::Base, brush);
    this->setPalette(paletteBGColor);

    this->repaint();
    this->update();
}

void RtabWidget::SetTableView(QTableView& tv, RModel& mm, int myltiplier  )
{
    // Ширина колонок
    for (auto cw : mm.ColumnsWidth())
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*myltiplier);
}
void RtabWidget::SetTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier  )
{
    view->tableOptions().setColumnAutoWidth(false);
    // Ширина колонок
    for (auto cw : mm.ColumnsWidth())
        tv.getColumn(std::get<0>(cw))->setWidth(std::get<1>(cw)*myltiplier);
}

// QTitanGrid: ContextMenu
void RtabWidget::contextMenu(ContextMenuEventArgs* args)
{
    column = args->hitInfo().columnIndex();
    row = args->hitInfo().row().rowIndex();
    QString qstr_col_props = "";
    if (column >= 0)
    {
        RCol* prcol = prm->getRCol(column);
        std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| -(" + prcol->name() + "), [" +prcol->unit() + "]";
        qstr_col_props = str_col_prop.c_str();
    }
    QAction* condFormatAction = new QAction(QIcon(":/icons/edit_cond_formats"), tr("Edit Conditional Formats..."),  args->contextMenu());

    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(qstr_col_props, this, SLOT(OpenColPropForm()));

    std::tuple<int,double> item_sum = GetSumSelected();
    args->contextMenu()->addAction("Sum: " + QString::number(std::get<1>(item_sum))+" Items: " + QString::number(std::get<0>(item_sum)),this,SLOT());
    //args->contextMenu()->addAction(tr("Скрыть колонку"),this,SLOT(hideColumns()));
    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr("Insert Row"),this,SLOT(insertRow_qtitan()),QKeySequence(Qt::CTRL | Qt::Key_I));
    args->contextMenu()->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),tr("Delete Row"),this,SLOT(deleteRow_qtitan()),QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(sC_CTRL_I, &QShortcut::activated, this, &RtabWidget::insertRow_qtitan);
    connect(sC_CTRL_D, &QShortcut::activated, this, &RtabWidget::deleteRow_qtitan);
    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction(tr("Выравнивание: по шаблону"),this,SLOT(widebyshabl()));
    args->contextMenu()->addAction(tr("Выравнивание: по данным"),this,SLOT(widebydata()));
    args->contextMenu()->addSeparator();
    args->contextMenu()->addAction("Экспорт CSV", this, SLOT(OpenExportCSVForm()));
    args->contextMenu()->addAction("Импорт CSV", this, SLOT(OpenImportCSVForm()));
    args->contextMenu()->addAction("Выборка", this, SLOT(OpenSelectionForm()));
    args->contextMenu()->addAction(condFormatAction);

    //connect(sC_CTRL_I, &QShortcut::activated, this, &RtabWidget::insertRow);
    //connect(sC_CTRL_D, &QShortcut::activated, this, &RtabWidget::deleteRow);

    QMenu *menu_connected_forms;
    menu_connected_forms = CunstructLinkedFormsMenu( stringutils::cp1251ToUtf8(m_UIForm.Name()));
    args->contextMenu()->addMenu(menu_connected_forms);

    connect(condFormatAction, &QAction::triggered, this, [&]() {
        emit editCondFormats(column);
    });
}

void RtabWidget::customMenuRequested(QPoint pos){
    index=ptv->indexAt(pos);

    MenuRequestedPoint = pos;

    QMenu *menu=new QMenu(this);
    QAction* copyAction = new QAction( tr("Copy"), menu);
    QAction* copyWithHeadersAction = new QAction( tr("Copy with Headers"), menu);
    QAction* condFormatAction = new QAction(QIcon(":/icons/edit_cond_formats"), tr("Edit Conditional Formats..."), menu);

    std::tuple<int,double> item_sum = GetSumSelected();
    menu->addAction("Sum: " + QString::number(std::get<1>(item_sum))+" Items: " + QString::number(std::get<0>(item_sum)),this,SLOT());
    menu->addSeparator();
    menu->addAction(copyAction);
    menu->addAction(copyWithHeadersAction);
    menu->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr("Insert Row"),this,SLOT(insertRow()),QKeySequence(Qt::CTRL | Qt::Key_I));
    menu->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),tr("Delete Row"),this,SLOT(deleteRow()),QKeySequence(Qt::CTRL | Qt::Key_D));
    //menu->addAction(tr("Hide Rows"),this,SLOT(hideRows()));
    //menu->addAction(tr("Unhide Rows"),this,SLOT(unhideRows()));
    menu->addSeparator();
    //menu->addAction(tr("Insert Columns"),this,SLOT(insertColumns()));
    //menu->addAction(tr("Delete Columns"),this,SLOT(deleteColumns()));
    //menu->addAction(tr("Hide Columns"),this,SLOT(hideColumns()));
    //menu->addAction(tr("Unhide Columns"),this,SLOT(unhideColumns()));
    menu->addAction(tr("Выравнивание: по шаблону"),this,SLOT(widebyshabl()));
    menu->addAction(tr("Выравнивание: по данным"),this,SLOT(widebydata()));
    menu->addSeparator();
    menu->addAction("Выборка", this, SLOT(OpenSelectionForm()));
    menu->addAction(tr("Format"));
    menu->addAction(condFormatAction);
    menu->popup(ptv->viewport()->mapToGlobal(pos));

    //ShotCuts (no ru variant :(  ...)
   // QShortcut *sC_CTRL_I = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_I), this);
   // QShortcut *sC_CTRL_D = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(sC_CTRL_I, &QShortcut::activated, this, &RtabWidget::insertRow);
    connect(sC_CTRL_D, &QShortcut::activated, this, &RtabWidget::deleteRow);

    //connect(copyAction, &QAction::triggered, this, &RtabWidget::copy);
    connect(copyAction, &QAction::triggered, this, [&]() {
        copy(false, false);
    });
    //connect(cutAction, &QAction::triggered, this, &ExtendedTableWidget::cut);
    connect(copyWithHeadersAction, &QAction::triggered, this, [&]() {
        copy(true, false);
    });

    connect(condFormatAction, &QAction::triggered, this, [&]() {
        emit editCondFormats(index.column());
    });

}

void RtabWidget::customHeaderMenuRequested(QPoint pos){
    MenuRequestedPoint = pos;
    column=ptv->horizontalHeader()->logicalIndexAt(pos);

    RCol* prcol = prm->getRCol(column);
    std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| -(" + prcol->name() + "), [" +prcol->unit() + "]";
    QString qstr_col_props = str_col_prop.c_str();

    QMenu *menu=new QMenu(this);
    menu->addAction(qstr_col_props, this, SLOT(OpenColPropForm()));
    menu->addSeparator();
    menu->addAction(QIcon(":/images/sortasc.png"),tr("sortAscending"), this, SLOT(sortAscending()));
    menu->addAction(QIcon(":/images/sortdesc.png"),tr("sortDescending"), this, SLOT(sortDescending()));
    menu->addSeparator();
    menu->addAction(tr("Clear Contents"),this,SLOT(clearContents()));
    menu->addSeparator();
    menu->addAction(tr("Скрыть"),this,SLOT(hideColumns()));
    menu->addAction(tr("Выбор колонок"),this,SLOT(unhideColumns()));
    menu->addAction(tr("Показать все колонки"),this,SLOT(showAllColumns()));
    menu->addSeparator();
    menu->addAction(tr("Format"));
    menu->popup(ptv->horizontalHeader()->viewport()->mapToGlobal(pos));
}

QMenu* RtabWidget::CunstructLinkedFormsMenu(std::string form_name)
{
    QMenu *menu=new QMenu(this);
    menu->setTitle("Связанные формы");

    std::vector<int> vbindvals;
    auto table_context_form = m_pRTDM->Get("formcontext","");

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
    //prtw->SetLinkedForm(_lf);

    // connect(button, &QPushButton::clicked, [this, text] { clicked(text); });
    //connect(view, &GridTableView::cellClicked, [this] { FillBindVals(); });

    _lf.conn = connect(this->view, SIGNAL(cellClicked( CellClickEventArgs* )), prtw,
            SLOT(onLinkedFormUpdate( CellClickEventArgs*)));

    prtw->SetLinkedForm(_lf);

    auto dw = new ads::CDockWidget( stringutils::cp1251ToUtf8(pUIForm->Name()).c_str(), this);
    dw->setWidget(prtw->m_grid);
    connect( dw, SIGNAL( closed() ),
            prtw, SLOT( OnClose() ) );                    // emit RtabWidget->closeEvent

    auto area = m_DockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    prtw->show();
}

void RtabWidget::cornerButtonPressed()
{
    auto hh = ptv->horizontalHeader();
    if(ptv->horizontalHeader()->count() < 1)
    return;

    if(!ptv->model())
        return;

    QObject::disconnect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                        this, SLOT(changeColumnVisible(QListWidgetItem*)));

    customizeListWidget.clear();

    QVector<QString> headers(ptv->model()->columnCount());
    QVector<bool> visibles(ptv->model()->columnCount());

    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            int x = hh->visualIndex(ix);
            headers[x] = ptv->model()->headerData(ix, Qt::Horizontal).toString();
            visibles[x] = hh->isSectionHidden(ix);
        }
    }
    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
            customizeListWidget.addItem(headers[ix]);
    }

    int x = 0;
    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            QListWidgetItem *item = customizeListWidget.item(x++);
            item->setData(RTABLEVIEWCOLINDEXROLE, ix);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(visibles[ix] ? Qt::Unchecked : Qt::Checked);
        }
    }

    QObject::connect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                     this, SLOT(changeColumnVisible(QListWidgetItem*)));

    //QPoint P(10, rect().y() + hdr->sectionHeight());
    QPoint P(10, rect().y() + hh->height());

    customizeFrame.move(mapToGlobal(P));
    customizeFrame.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customizeFrame.show();

    emit onCornerButtonPressed();

}

void RtabWidget::changeColumnVisible(QListWidgetItem *item)
{
    //PBSHeaderView *hdr = static_cast<PBSHeaderView*>(pTableWidget.horizontalHeader());
    /*
     * Чтобы при загрузке файла сохранялась форма нужно
     * редактировать m_UIForm при изменении настроек отображения стобцов
     * --   Работает, но теперь вопрос если загрузили файл в котором нет
     * исходных полей, при попытке изменить значение ошибка конечно же
    */

    auto hh = this->ptv->horizontalHeader();

    int ix = item->data(RTABLEVIEWCOLINDEXROLE).toInt();
    int i = hh->logicalIndex(ix); // customizeListWidget.row(item));

    std::string name = (item->data(Qt::DisplayRole).toString()).toStdString();
    std::string item_rname = item->whatsThis().toStdString();
    if(i < 0)
        return;

    if(item->checkState() == Qt::Checked)
    {
        tItemStateMap[ix].bHidden = true;
        if(hh->isSectionHidden(i))
        {
            hh->showSection(i);
            CUIFormField uiff(item_rname.c_str());
            m_UIForm.Fields().insert( m_UIForm.Fields().begin(),uiff);
        }
    }
    else
    {
        tItemStateMap[ix].bHidden = false;
        if(!hh->isSectionHidden(i))
        {
            hh->hideSection(i);
            m_UIForm.Fields().remove_if([&](const CUIFormField Field){return (Field.Name() == item_rname);});
        }
    }
}

void RtabWidget::editCondFormats(size_t column)
{
    std::vector<CondFormat> condFormats;
    CondFormat condFormat;
   // CondFormatManager condFormatDialog(m_settings[currentlyBrowsedTableName()].condFormats[column],
    //                                  m_model->encoding(), this);
    CondFormatManager condFormatDialog(m_MapcondFormatVector[column],
                                            "UTF-8", this);
    //CondFormatManager condFormatDialog(condFormats,
    //                                  "UTF-8", this);

    QString title= prm->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString();
    condFormatDialog.setWindowTitle(tr("Conditional formats for \"%1\"").
                                    arg(prm->headerData(static_cast<int>(column), Qt::Horizontal, Qt::DisplayRole).toString()));
    if (condFormatDialog.exec()) {
        std::vector<CondFormat> condFormatVector = condFormatDialog.getCondFormats();
        prm->setCondFormats(false, column, condFormatVector);
        /*if (m_MapcondFormatVector.find(static_cast<int>(column)) != m_MapcondFormatVector.end())
            m_MapcondFormatVector.at(static_cast<int>(column)) = condFormatVector;
        else
            m_MapcondFormatVector.insert(std::pair(static_cast<int>(column),condFormatVector));
        */
        m_MapcondFormatVector.at(column) = condFormatVector;

        //nlohmann::json j = nlohmann::json::parse(condFormatVector[0]);
       /* nlohmann::json j;
        j = {
            {"node",title.toStdString().c_str(),condFormatVector[0].filter().toStdString()}
        };
        std::string jstr = j.dump();
        std::filesystem::path path_2_json = std::filesystem::current_path() / "HighLightSetting.json";
        std::ofstream ofs(path_2_json);
        if(ofs.is_open())
        {
            std::string  str_file = j.dump(1, ' ');
            ofs << str_file;
            ofs.close();
        }
        */

        //m_settings[currentlyBrowsedTableName()].condFormats[column] = condFormatVector;
        emit CondFormatsModified();
    }
}
void RtabWidget::onCondFormatsModified()
{
    CondFormatJson cfj(prm->getRdata()->t_name_ , prm->getRdata()->vCols_ ,m_MapcondFormatVector );
    cfj.save_json();
}

void RtabWidget::onLinkedFormUpdate( CellClickEventArgs* _args)
{
    int row = _args->cell().rowIndex();
    int col = _args->cell().columnIndex();
    qDebug()<<"Linked form catch Pressed:" <<row<< ","<<col;

    m_lf.row = row;
    m_lf.FillBindVals();
    SetLinkedForm(m_lf);
}
void RtabWidget::onItemPressed( CellClickEventArgs* _args)
{
    int row = _args->cell().rowIndex();
    int col = _args->cell().columnIndex();
    qDebug()<<"Pressed:" <<row<< ","<<col;
}
void RtabWidget::onItemPressed(const QModelIndex &_index)
{
    index = _index;
    int row = index.row();
    int column = index.column();
    qDebug()<<"Pressed:" <<row<< ","<<column;
}
void RtabWidget::insertRow()
{
    prm->insertRows(index.row(),1,index);
}
void RtabWidget::insertRow_qtitan()
{
    GridSelection* selection = view->selection();
    int row = selection->cell().rowIndex();
    QModelIndex index = selection->cell().modelIndex();

    view->beginUpdate();
    prm->insertRows(row,1,index);
    view->endUpdate();

    this->update(0,0,1000,1000);
    this->repaint(0,0,1000,1000);
}
void RtabWidget::deleteRow()
{
    prm->removeRows(index.row(),1,index);
}
void RtabWidget::deleteRow_qtitan()
{
    GridSelection* selection = view->selection();
    int row = selection->cell().rowIndex();
    QModelIndex index = selection->cell().modelIndex();

    prm->removeRows(row,1,index);
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
    ColPropForm* PropForm = new ColPropForm(prm->getRdata(),ptv, prcol);
    PropForm->show();
}
void RtabWidget::OpenSelectionForm()
{
    FormSelection* Selection = new FormSelection(this->m_selection, this);
    Selection->show();
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
    formimportcsv* ImportCsv = new formimportcsv( prm->getRdata(),this);
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
void RtabWidget::unhideColumns()
{
    RCol* prcol = prm->getRCol(column);
    auto hh = ptv->horizontalHeader();

    QObject::disconnect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                        this, SLOT(changeColumnVisible(QListWidgetItem*)));

    customizeListWidget.clear();

    QVector<QString> headers(ptv->model()->columnCount());
    QVector<QString> rnames(ptv->model()->columnCount());
    QVector<bool> visibles(ptv->model()->columnCount());

    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            int x = hh->visualIndex(ix);
            headers[x] = ptv->model()->headerData(ix, Qt::Horizontal).toString();
            rnames[x] = prm->getRCol(ix)->str_name_.c_str();
            visibles[x] = hh->isSectionHidden(ix);
        }
    }
    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
            customizeListWidget.addItem(headers[ix]);
    }

    int x = 0;
    for(int ix = 0; ix < hh->count(); ix++)
    {
        if(tItemStateMap[ix].bVisible)
        {
            QListWidgetItem *item = customizeListWidget.item(x++);
            item->setData(RTABLEVIEWCOLINDEXROLE, ix);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(visibles[ix] ? Qt::Unchecked : Qt::Checked);
            item->setWhatsThis(rnames[ix]);
        }
    }

    QObject::connect(&customizeListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                     this, SLOT(changeColumnVisible(QListWidgetItem*)));


    int position = hh->sectionPosition(prcol->index);
    //QPoint P(point.x(), point.y() + 20);
    QPoint P(position, rect().y() + hh->height());


    customizeFrame.move(mapToGlobal(P));
    //customizeFrame.move(P);
    customizeFrame.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customizeFrame.show();
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
                result.append(headerText);
                htmlResult.append(headerText);
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
    result.removeLast();
    QString test = "test str\r\n";
    QString test2 = "test str\r\n";
    test.removeLast();
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




