#ifndef RTABWIDGET_H
#define RTABWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QMenu>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include "ColPropForm.h"
#include "rmodel.h"

class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(QWidget *parent = nullptr);
    explicit RtabWidget(CRastrHlp& prh,int n_indx,QWidget *parent = nullptr);
    void SetTableView(QTableView& tv, RModel& mm);

public slots:
    void customMenuRequested(QPoint pos);
    void customHeaderMenuRequested(QPoint pos);
    void onItemPressed(const QModelIndex &index);
    void insertRow();
    void deleteRow();
    void OpenColPropForm();


private:
    QTableView* ptv ;
    RModel *prm;

    QModelIndex index;              // current index (Cell clicked)
    int column;                     // for header

signals:
};

#endif // RTABWIDGET_H
