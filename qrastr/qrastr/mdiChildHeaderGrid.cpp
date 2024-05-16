#include "mdiChildHeaderGrid.h"

#if(!defined(QICSGRID_NO))

#include "mdiChildGrid.h"

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
    return (new mdiChildHeaderGrid(w, info, type));
}

void mdiChildHeaderGrid::handleMousePressEvent(const QicsICell &cell, QMouseEvent *m)
{
    //learn : QSignalMapper
    //https://forum.qt.io/topic/118635/cannot-pass-parameter-in-slot

    MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
    table->ind_col_clicked = cell.column();
    if (m->button() == Qt::RightButton) {
    if (!m_menu) {
            m_menu = new QMenu(this);

            m_menu->addAction(QIcon(":/images/sortasc.png"),tr("sortAscending"), table, SLOT(sortAscending()));
            m_menu->addAction(QIcon(":/images/sortdesc.png"),tr("sortDescending"), table, SLOT(sortDescending()));

            m_menu->addSeparator();
            m_menu->addAction(tr("Clear Contents"),table,SLOT(clearContents()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Hide Columns"),table,SLOT(hideColumns()));
            m_menu->addAction(tr("Unhide Columns"),table,SLOT(unhideColumns()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Format"));
        }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_menu->exec(m->globalPos());
#else
        m_menu->exec(m->globalPosition().toPoint());
#endif
        m->accept();
    }
    else if (m->button() == Qt::LeftButton)
    {
        MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
        int col_ind = cell.column();
        table->sort_by_col(col_ind);
    }
    else
        QicsHeaderGrid::handleMousePressEvent(cell, m);


}

#endif //#if(!defined(QICSGRID_NO))
