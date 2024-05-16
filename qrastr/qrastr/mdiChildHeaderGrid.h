#ifndef MDICHILDHEADERGRID_H
#define MDICHILDHEADERGRID_H

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

    private:
        QMenu *m_menu;

};


#endif // MDICHILDHEADERGRID_H
