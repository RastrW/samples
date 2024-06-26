#ifndef MDICHILDHEADERGRID_H
#define MDICHILDHEADERGRID_H

#if(!defined(QICSGRID_NO))

#include <QicsHeaderGrid.h>

class QMenu;
class QicsSelection;

class mdiChildHeaderGrid : QicsHeaderGrid
{
    Q_OBJECT
public:
    //mdiChildHeaderGrid();
    mdiChildHeaderGrid(QWidget *w, QicsGridInfo &info,
            Qics::QicsHeaderType type);

        static QicsHeaderGrid *createHeaderGrid(QWidget *w, QicsGridInfo &info,
            Qics::QicsHeaderType type);

protected:
        virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);
        virtual void handleMouseDoubleClickEvent(const QicsICell &cell, QMouseEvent *m);

private:
        QMenu *m_menu;

public slots:
    void AutoWidthColumns();
};

#endif //#if(!defined(QICSGRID_NO))

#endif // MDICHILDHEADERGRID_H
