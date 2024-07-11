#include "rtableview.h"
#include "FilterTableHeader.h"

RTableView::RTableView()
{
    // Set up filter row
    m_tableHeader = new FilterTableHeader(this);
    m_tableHeader->generateFilters(10);
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца

}
