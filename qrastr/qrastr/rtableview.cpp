#include "rtableview.h"
#include "rtabwidget.h"
#include "FilterTableHeader.h"
#include <QScrollBar>

RTableView::RTableView()
{
    // Set up filter row
    m_tableHeader = new FilterTableHeader(this);
    //m_tableHeader->setFilter(4,tr("<100"));
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца

    //connect(verticalScrollBar(), &QScrollBar::valueChanged,  dynamic_cast<RtabWidget*>(this->parentWidget()), &RtabWidget::vscrollbarChanged);

}
/*RTableView::RTableView(RTabWidget* rtw)
{
    m_tableHeader = new FilterTableHeader(this);
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца

    connect(verticalScrollBar(), &QScrollBar::valueChanged,  dynamic_cast<RtabWidget*>(this->parentWidget()), &RtabWidget::vscrollbarChanged);
}*/
void RTableView::generateFilters(int count)
{
    m_tableHeader->generateFilters(count);
}


