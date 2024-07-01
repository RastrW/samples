#include "rtabwidget.h"
#include <QVBoxLayout>

//#include "tableview.h"


RtabWidget::RtabWidget(QWidget *parent)
    : QWidget{parent}
{
}

RtabWidget::RtabWidget(CRastrHlp& rh,int n_indx, QWidget *parent)
    : QWidget{parent}
{
    ptv = new QTableView();
    ptv->setContextMenuPolicy(Qt::CustomContextMenu);                   //https://forum.qt.io/topic/31233/how-to-create-a-custom-context-menu-for-qtableview/6
    ptv->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ptv, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customMenuRequested(QPoint)));
    connect(ptv, SIGNAL(pressed(const QModelIndex &)),
            SLOT(onItemPressed(const QModelIndex &)));
    connect(ptv->horizontalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customHeaderMenuRequested(QPoint)));

    prm = new RModel(nullptr, rh );

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(prm); // used for sorting: create proxy //https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    // proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(prm);
    prm->setFormIndx(n_indx);
    prm->populateDataFromRastr();

    ptv->setSortingEnabled(true);
    ptv->setModel(proxyModel);
    ptv->resize(1000,500);
    SetTableView(*ptv,*prm);
    ptv->setParent(this);

    ptv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ptv);
    setLayout(layout);
}

void RtabWidget::SetTableView(QTableView& tv, RModel& mm)
{
    // Ширина колонок
    int myltiplier = 15;
    for (auto cw : mm.ColumnsWidth())
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*myltiplier);
}

void RtabWidget::customMenuRequested(QPoint pos){
    index=ptv->indexAt(pos);

    QMenu *menu=new QMenu(this);
    //menu->addAction(QIcon(":/images/cut.png"),tr("Cut"), this, SLOT(cut()));
    //menu->addAction(QIcon(":/images/copy.png"),tr("Copy"), this, SLOT(copy()));
    //menu->addAction(QIcon(":/images/paste.png"),tr("Paste"), this, SLOT(paste()));
    //menu->addSeparator();
    //menu->addAction(tr("Clear Contents"),this,SLOT(clearContents()));
    //menu->addSeparator();
    menu->addAction(tr("Insert Row"),this,SLOT(insertRow()));
    menu->addAction(tr("Delete Row"),this,SLOT(deleteRow()));
    //menu->addAction(tr("Hide Rows"),this,SLOT(hideRows()));
    //menu->addAction(tr("Unhide Rows"),this,SLOT(unhideRows()));
    menu->addSeparator();
    //menu->addAction(tr("Insert Columns"),this,SLOT(insertColumns()));
    //menu->addAction(tr("Delete Columns"),this,SLOT(deleteColumns()));
    menu->addAction(tr("Hide Columns"),this,SLOT(hideColumns()));
    menu->addAction(tr("Unhide Columns"),this,SLOT(unhideColumns()));
    menu->addSeparator();
    menu->addAction(tr("Format"));
    menu->popup(ptv->viewport()->mapToGlobal(pos));
}

void RtabWidget::customHeaderMenuRequested(QPoint pos){
    column=ptv->horizontalHeader()->logicalIndexAt(pos);

    RCol* prcol = prm->getRCol(column);
    std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| " + prcol->name() + ", [" +prcol->unit() + "]";
    QString qstr_col_props = str_col_prop.c_str();

    QMenu *menu=new QMenu(this);
    menu->addAction(qstr_col_props, this, SLOT(OpenColPropForm()));
    menu->addSeparator();
    menu->addAction(QIcon(":/images/sortasc.png"),tr("sortAscending"), this, SLOT(sortAscending()));
    menu->addAction(QIcon(":/images/sortdesc.png"),tr("sortDescending"), this, SLOT(sortDescending()));
    menu->addSeparator();
    menu->addAction(tr("Clear Contents"),this,SLOT(clearContents()));
    menu->addSeparator();
    menu->addAction(tr("Hide Columns"),this,SLOT(hideColumns()));
    menu->addAction(tr("Unhide Columns"),this,SLOT(unhideColumns()));
    //m_menu->addAction(tr("Подбор ширины все колонки"),this,SLOT(AutoWidthColumns())); // thats  not work
    //m_menu->addAction(Act_AutoWidthColumns);
    menu->addSeparator();
    menu->addAction(tr("Format"));
    menu->popup(ptv->horizontalHeader()->viewport()->mapToGlobal(pos));
}
void RtabWidget::onItemPressed(const QModelIndex &index)
{
    int row = index.row();
    int column = index.column();
    //QTableView* t = index.parent();
    qDebug()<<"Pressed:" <<row<< ","<<column;
}
void RtabWidget::insertRow()
{
#if(!defined(QICSGRID_NO))
    int rowIndex = activeMdiChild()->currentCell()->rowIndex();
    if (rowIndex < 0)
        return;
    activeMdiChild()->insertRow( rowIndex );
#endif//#if(!defined(QICSGRID_NO))
#if(defined(QICSGRID_NO))

    prm->insertRows(index.row(),1,index);

#endif//#if(defined(QICSGRID_NO))

}
void RtabWidget::deleteRow()
{
    prm->removeRows(index.row(),1,index);
}

void RtabWidget::OpenColPropForm()
{
    RCol* prcol = prm->getRCol(index.column());
    ColPropForm* PropForm = new ColPropForm(prm->getRdata(),prcol);
    PropForm->show();
}
