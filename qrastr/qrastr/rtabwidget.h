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
class QAstra;

class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(QWidget *parent = nullptr);
    explicit RtabWidget(QAstra* pqastra, CUIForm UIForm, QWidget *parent = nullptr);

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
    void onFileLoad();
    void update_data();

private slots:
    void CreateModel(QAstra* pqastra,CUIForm* pUIForm);
    void onRastrHint(const _hint_data&);

public:
    RModel* prm;
    CUIForm m_UIForm;
    QAstra* m_pqastra;

private:
    using BufferRow = std::vector<QByteArray>;
    std::vector<std::vector<QByteArray>> m_buffer;
    QString m_generatorStamp;
    RTableView* ptv ;
    QSortFilterProxyModel *proxyModel;
    QModelIndex index;              // current index (Cell clicked)
    int form_indx;
    int column;                     // for header

signals:

};

#endif // RTABWIDGET_H
