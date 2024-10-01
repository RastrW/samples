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

typedef enum {ifNone, ifCheckBox } modelFlag;
typedef enum {skNone = 0, skCount = 1, skSum = 2, skMin = 3, skMax = 4, skAvg = 5} footerEvent;
typedef enum {etLineEdit, etMemo, etCheckBox, etIntSpin, etDoubleSpin, etDate, etDateTime, etProgress, etButton, etPicture } itemEditType;

struct ItemState
{
    Qt::Alignment aAlignColumn;
    Qt::Alignment aAlignFooter;

    itemEditType editType;

    bool bSortable;
    bool bFiltered;
    bool bHidden;
    bool bVisible;
    bool bEditable;
    bool bSelectable;

    QString sFilter;
    QString sFilterBoxFilter;
    QString sFormat;
    QString sFooterFormat;
    QString sCaption;

    footerEvent fFooterEvent;

    QLabel *footerLabel;
    QVariant footerValue;

    ItemState()
    {
        aAlignColumn = aAlignFooter = Qt::AlignRight | Qt::AlignVCenter;

        editType = etLineEdit;

        bSortable = bFiltered = bVisible = bEditable = bSelectable = true;
        bHidden = false;

        sFilter = "";
        sFilterBoxFilter = "";
        sFormat = "";
        sFooterFormat = "#,#0.#0";

        fFooterEvent = skNone;

        footerLabel = nullptr;
        footerValue = QVariant();
    }

    void copyFrom(ItemState its)
    {
        aAlignColumn = its.aAlignColumn;
        aAlignFooter = its.aAlignFooter;

        editType = its.editType;

        bSortable = its.bSortable;
        bFiltered = its.bFiltered;
        bVisible  = its.bVisible;
        bEditable = its.bEditable;
        bSelectable = its.bSelectable;
        bHidden   = its.bHidden;

        sFilter = its.sFilter;
        sFilterBoxFilter = its.sFilterBoxFilter;
        sFormat = its.sFormat;
        sFooterFormat = sFooterFormat;

        fFooterEvent = its.fFooterEvent;
    }
};

typedef QMap<int, ItemState> itemStateMap;

#define RTABLEVIEWCOLINDEXROLE Qt::UserRole + 1

class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(QWidget *parent = nullptr);
    explicit RtabWidget(QAstra* pqastra, CUIForm UIForm, QWidget *parent = nullptr);

    void SetTableView(QTableView& tv, RModel& mm, int myltiplier = 10);
private:
    void test(const QModelIndexList& fromIndices);
    void copyMimeData(const QModelIndexList& fromIndices, QMimeData* mimeData, const bool withHeaders, const bool inSQL);
    void copy(const bool withHeaders, const bool inSQL);
    void copy();
signals:
    void onCornerButtonPressed();
public slots:
    void customMenuRequested(QPoint pos);
    void customHeaderMenuRequested(QPoint pos);
    void onItemPressed(const QModelIndex &index);
    void changeColumnVisible(QListWidgetItem*);
    void cornerButtonPressed();
    void insertRow();
    void deleteRow();
    void widebyshabl();
    void widebydata();
    void OpenColPropForm();
    void sortAscending();
    void sortDescending();
    void hideColumns();
    void unhideColumns();
    void showAllColumns();
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
    QPoint MenuRequestedPoint;
protected:
    itemStateMap tItemStateMap;
    QFrame customizeFrame;
    QListWidget customizeListWidget;

signals:

};

#endif // RTABWIDGET_H
