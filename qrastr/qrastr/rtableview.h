#ifndef RTABLEVIEW_H
#define RTABLEVIEW_H

#include <QTableView>

class FilterTableHeader;
class RTabWidget;

class RTableView : public QTableView
{
    Q_OBJECT
public:
    RTableView();
    //RTableView(RTabWidget* rtw);
    void generateFilters(int count);

protected:
     FilterTableHeader* m_tableHeader;
};

#endif // RTABLEVIEW_H
