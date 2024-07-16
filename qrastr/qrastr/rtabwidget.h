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
#include "RtableView.h"

class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(QWidget *parent = nullptr);
    explicit RtabWidget(CRastrHlp& _rh,int n_indx,QWidget *parent = nullptr);
    void SetTableView(QTableView& tv, RModel& mm);

public slots:
    void customMenuRequested(QPoint pos);
    void customHeaderMenuRequested(QPoint pos);
    void onItemPressed(const QModelIndex &index);
    void insertRow();
    void deleteRow();
    void OpenColPropForm();
    void sortAscending();
    void sortDescending();
    void onUpdate(std::string _t_name);
    void updateFilter(size_t column, QString value);
    void onFileLoad(CRastrHlp& _rh);

//private slots:


public:
    RModel *prm;
private:
    //QTableView* ptv ;
   // CRastrHlp& rh;
    RTableView* ptv ;
    QSortFilterProxyModel *proxyModel;
    QModelIndex index;              // current index (Cell clicked)
    int form_indx;
    int column;                     // for header

signals:

};

#endif // RTABWIDGET_H
