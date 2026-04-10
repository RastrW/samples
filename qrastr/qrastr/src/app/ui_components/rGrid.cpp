#include "rGrid.h"
#include "rGridTableView.h"

Qtitan::GridViewBase* RGrid::createView(GridViewType type)
{
    switch (type)
    {
    case TableView:
        return new RGridTableView(this);
    case TableViewVertical:
        return new Qtitan::GridTableViewVertical(this);
    default:
        return Qtitan::Grid::createView(type);
    }
}