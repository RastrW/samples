#include "rGridTableView.h"

RGridTableView::RGridTableView(Qtitan::GridBase* grid)
        : GridTableView(grid) {}

void RGridTableView::columnWidthChanged(Qtitan::GridColumnBase* column)
{
	GridTableView::columnWidthChanged(column);
	emit sig_columnWidthChanged(column);
}