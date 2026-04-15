#include "rGridTableView.h"

RGridTableView* RGridTableView::create(Qtitan::GridBase* grid)
{
    RGridTableView* view = new RGridTableView(grid);
    view->initialize();   // protected — доступен из статического метода своего класса
    return view;
}

RGridTableView::RGridTableView(Qtitan::GridBase* grid)
    : Qtitan::GridTableView(grid) {}

void RGridTableView::columnWidthChanged(Qtitan::GridColumnBase* column)
{
    Qtitan::GridTableView::columnWidthChanged(column);
    emit sig_columnWidthChanged(column);
}