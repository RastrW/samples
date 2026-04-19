#include "rGrid.h"
#include "rGridTableView.h"

RGrid::RGrid(QWidget* parent)
    : Qtitan::Grid(parent)
{}

Qtitan::GridViewBase* RGrid::createView(GridViewType type)
{
    switch (type)
    {
    case TableView:
        return RGridTableView::create(this);
    case TableViewVertical:
        return Qtitan::Grid::createView(type);
    default:
        return Qtitan::Grid::createView(type);
    }
}