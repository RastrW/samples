#ifndef MDICHILDGRID_H
#define MDICHILDGRID_H

#include <QicsTableGrid.h>

/*
* Custom grid. Provides context menu with default editing actions.
* Also performs editing of big data sets with disabled repaint(RepaintBehavior = Off)
*/

class QMenu;
class QicsSelection;

class mdiChildGrid : public QicsTableGrid
{
    Q_OBJECT
public:
    mdiChildGrid(QWidget *w, QicsGridInfo &info,
            int top_row = 0, int left_column = 0);

        static QicsTableGrid *createGrid(QWidget *w, QicsGridInfo &info,
            int top_row = 0, int left_column = 0);

    protected:
        virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);

    private:
        QMenu *m_menu;

};

#endif // MDICHILDGRID_H
