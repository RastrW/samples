#ifndef RTABLEVIEW_H
#define RTABLEVIEW_H

#include <QTableView>

class FilterTableHeader;
class RTableView : public QTableView
{
    Q_OBJECT
public:
    RTableView();

protected:
     FilterTableHeader* m_tableHeader;
};

#endif // RTABLEVIEW_H
