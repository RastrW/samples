#include "mdiChildHeaderGrid.h"

#if(!defined(QICSGRID_NO))
#include "mdiChildGrid.h"
#include "rastrdatamodel.h"

#include <QMouseEvent>
#include <QMenu>

#include <QicsCell.h>
#include <QicsSelection.h>
#include <QicsRow.h>
#include <QicsColumn.h>

#include "mdiChildTable.h"
#include "mainwindow.h"
//QicsHeaderGrid(QWidget *w, QicsGridInfo &info, Qics::QicsHeaderType type);
mdiChildHeaderGrid::mdiChildHeaderGrid(QWidget *w, QicsGridInfo &info,
                           Qics::QicsHeaderType type)
: QicsHeaderGrid(w, info, type)
{
    m_menu = 0;


}

QicsHeaderGrid *mdiChildHeaderGrid::createHeaderGrid(QWidget *w, QicsGridInfo &info,
                                           Qics::QicsHeaderType type)
{
    auto prf = info.rowFilter();
    //type = Qics::QicsHeaderType::RowHeader;
    //info.insertRow(0);
    //prf->setFilter()
    QicsHeaderGrid* pHG = new mdiChildHeaderGrid(w, info, type);

    return pHG;
}

void mdiChildHeaderGrid::handleMousePressEvent(const QicsICell &cell, QMouseEvent *m)
{
    //learn : QSignalMapper
    //https://forum.qt.io/topic/118635/cannot-pass-parameter-in-slot

    MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
    table->ind_col_clicked = cell.column();
    RCol* prcol = table->GetCol();

    QAction *Act_AutoWidthColumns = new QAction();
    Act_AutoWidthColumns->setStatusTip(tr("Подбор ширины все колонки"));
    Act_AutoWidthColumns->setIconText("Подбор ширины все колонки");
    connect(Act_AutoWidthColumns, SIGNAL(triggered()), this, SLOT(AutoWidthColumns()));

    std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| " + prcol->name() + ", [" +prcol->unit() + "]";
    QString qstr_col_props = str_col_prop.c_str();

    if (m->button() == Qt::RightButton) {
    //if (!m_menu) {
            m_menu = new QMenu(this);
            m_menu->addAction(qstr_col_props, table, SLOT(OpenColPropForm()));
            m_menu->addSeparator();
            m_menu->addAction(QIcon(":/images/sortasc.png"),tr("sortAscending"), table, SLOT(sortAscending()));
            m_menu->addAction(QIcon(":/images/sortdesc.png"),tr("sortDescending"), table, SLOT(sortDescending()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Clear Contents"),table,SLOT(clearContents()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Hide Columns"),table,SLOT(hideColumns()));
            m_menu->addAction(tr("Unhide Columns"),table,SLOT(unhideColumns()));
            //m_menu->addAction(tr("Подбор ширины все колонки"),this,SLOT(AutoWidthColumns())); // thats  not work
            m_menu->addAction(Act_AutoWidthColumns);
            m_menu->addSeparator();
            m_menu->addAction(tr("Format"));
    //    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_menu->exec(m->globalPos());
#else
        m_menu->exec(m->globalPosition().toPoint());
#endif
        m->accept();
    }
    /*else if (m->button() == Qt::LeftButton)
    {
        MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
        int col_ind = cell.column();
        int h_row = cell.row();
        if (h_row == 0)
            table->sort_by_col(col_ind);
    }*/
    else
        QicsHeaderGrid::handleMousePressEvent(cell, m);
}

void mdiChildHeaderGrid::handleMouseDoubleClickEvent(const QicsICell &cell, QMouseEvent *m)
{
     QicsHeader hdr(&(gridInfo()));

    bool bch_size = isWithinResizeThreshold(m->x(), m->y());
    if (!bch_size)
    {
        if (!cell.isValid())
               return;

           const int row = cell.row();
           const int col = cell.column();
           MdiChild *table = qobject_cast<MdiChild*>(parentWidget());

           if (row == 0)
               table->sort_by_col(col);
    }
    else
        QicsHeaderGrid::handleMouseDoubleClickEvent(cell, m);
}




void mdiChildHeaderGrid::AutoWidthColumns()
{
    MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
    //table->autoFitMode();
    int num_cols= table->dataModel()->numColumns();
    for(int i = 0 ; i < num_cols; i++)
    {
        emit gripDoubleClicked(i,0,Qics::QicsHeaderType::ColumnHeader);
    }
}

#endif //#if(!defined(QICSGRID_NO))
