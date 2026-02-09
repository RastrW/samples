#if(!defined(QICSGRID_NO))

#include "mdiChildGrid.h"
#include <QMouseEvent>
#include <QMenu>
#include <QicsCell.h>
#include <QicsSelection.h>
#include <QicsRow.h>
#include <QicsColumn.h>
#include "mdiChildTable.h"

mdiChildGrid::mdiChildGrid(QWidget *w, QicsGridInfo &info,
                           int top_row, int left_column)
: QicsTableGrid(w, info, top_row, left_column)
{
m_menu = 0;
}

QicsTableGrid *mdiChildGrid::createGrid(QWidget *w, QicsGridInfo &info,
                                           int top_row, int left_column)
{
    return (new mdiChildGrid(w, info, top_row, left_column));
}

void mdiChildGrid::handleMousePressEvent(const QicsICell &cell, QMouseEvent *m)
{
    MdiChild *table = qobject_cast<MdiChild*>(parentWidget());
    table->ind_col_clicked = cell.column();

    if (m->button() == Qt::RightButton) {
    if (!m_menu) {
            //MdiChild *table = qobject_cast<MdiChild*>(parentWidget());

            m_menu = new QMenu(this);
            m_menu->addAction(QIcon(":/images/cut.png"),tr("Cut"), table, SLOT(cut()));
            m_menu->addAction(QIcon(":/images/copy.png"),tr("Copy"), table, SLOT(copy()));
            m_menu->addAction(QIcon(":/images/paste.png"),tr("Paste"), table, SLOT(paste()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Clear Contents"),table,SLOT(clearContents()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Insert Rows"),table,SLOT(insertRows()));
            m_menu->addAction(tr("Delete Rows"),table,SLOT(deleteRows()));
            m_menu->addAction(tr("Hide Rows"),table,SLOT(hideRows()));
            m_menu->addAction(tr("Unhide Rows"),table,SLOT(unhideRows()));
            m_menu->addSeparator();
            m_menu->addAction(tr("Insert Columns"),table,SLOT(insertColumns()));
            m_menu->addAction(tr("Delete Columns"),table,SLOT(deleteColumns()));
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
    } else
        QicsTableGrid::handleMousePressEvent(cell, m);
}

#endif //#if(!defined(QICSGRID_NO))
