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



class QMimeData;

class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(QWidget *parent = nullptr);
    explicit RtabWidget(CRastrHlp& _rh,int n_indx,QWidget *parent = nullptr);
    void SetTableView(QTableView& tv, RModel& mm, int myltiplier = 15);
private:
    void test(const QModelIndexList& fromIndices);
    void copyMimeData(const QModelIndexList& fromIndices, QMimeData* mimeData, const bool withHeaders, const bool inSQL);
    void copy(const bool withHeaders, const bool inSQL);
    void copy();

public slots:
    void customMenuRequested(QPoint pos);
    void customHeaderMenuRequested(QPoint pos);
    void onItemPressed(const QModelIndex &index);
    void insertRow();
    void deleteRow();
    void widebyshabl();
    void widebydata();
    void OpenColPropForm();
    void sortAscending();
    void sortDescending();
    void onUpdate(std::string _t_name);
    void updateFilter(size_t column, QString value);
    void onFileLoad(CRastrHlp& _rh);
    void update_data();

private slots:
    void CreateModel(CRastrHlp& _rh);


public:
    RModel *prm;
private:
    //QTableView* ptv ;
   // CRastrHlp& rh;
    using BufferRow = std::vector<QByteArray>;
    //static std::vector<std::vector<QByteArray>> m_buffer;
    std::vector<std::vector<QByteArray>> m_buffer;
    //static QString m_generatorStamp;
    QString m_generatorStamp;
    RTableView* ptv ;
    QSortFilterProxyModel *proxyModel;
    QModelIndex index;              // current index (Cell clicked)
    int form_indx;
    int column;                     // for header

signals:

};

#endif // RTABWIDGET_H
