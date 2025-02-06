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
#include "formselection.h"
#include "formgroupcorrection.h"
#include "formexportcsv.h"
#include "formimportcsv2.h"
#include "rmodel.h"
#include "rtableview.h"
#include "rtablesdatamanager.h"
#include "utils.h"
#include "linkedform.h"



namespace ads{ class CDockManager; }

class RtabWidget;
class QMimeData;
class QAstra;
class LinkedForm;


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
    explicit RtabWidget(QAstra* pqastra, CUIForm UIForm, RTablesDataManager* pRTDM, ads::CDockManager* pDockManager ,QWidget *parent = nullptr);
    /*~RtabWidget(){}; //C2036
    {
        //qDebug()<<"RtabWidget::Destructor "<< "[" <<m_UIForm.Name().c_str() << "]";
    };*/

    void SetTableView(QTableView& tv, RModel& mm, int myltiplier = 10);
    void SetTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier = 10 );
    void closeEvent(QCloseEvent* event) override;

private:
    void test(const QModelIndexList& fromIndices);
    void copyMimeData(const QModelIndexList& fromIndices, QMimeData* mimeData, const bool withHeaders, const bool inSQL);
    void copy(const bool withHeaders, const bool inSQL);
    void copy();
    std::tuple<int,double> GetSumSelected();
    QMenu* CunstructLinkedFormsMenu(std::string form_name);

signals:
    void onCornerButtonPressed();
    void CondFormatsModified();
public slots:
    void on_calc_begin();
    void on_calc_end();
    void OnClose();
    void onvisibilityChanged(bool visible);
    void contextMenu(ContextMenuEventArgs* args);
    void customMenuRequested(QPoint pos);
    void customHeaderMenuRequested(QPoint pos);

    void onItemPressed(const QModelIndex &index);
    void onItemPressed(CellClickEventArgs* _index);
    void onLinkedFormUpdate(CellClickEventArgs* _index);

    void changeColumnVisible(QListWidgetItem*);
    void cornerButtonPressed();
    void insertRow();
    void AddRow();
    void insertRow_qtitan();
    void DuplicateRow_qtitan();
    void deleteRow();
    void deleteRow_qtitan();
    void widebyshabl();
    void widebydata();
    void OpenColPropForm();
    void OpenSelectionForm();
    void OpenGroupCorrection();
    void OpenLinkedForm(std::string name,std::string selection , std::vector<int> keys );    // ТИ:Каналы ; id1=%d & id2=0 & prv_num<8 ; 801
    void OpenExportCSVForm();
    void OpenImportCSVForm();
    void sortAscending();
    void sortDescending();
    void hideColumns();
    void unhideColumns();
    void showAllColumns();
    void onUpdate(std::string _t_name);
    void updateFilter(size_t column, QString value);
    void onFileLoad();
    void update_data();
    void SetSelection(std::string Selection);
    void editCondFormats(size_t column);
    void onCondFormatsModified();
    void SetLinkedForm( LinkedForm _lf);
    void onOpenLinkedForm(LinkedForm _lf );    // ТИ:Каналы ; id1=%d & id2=0 & prv_num<8 ; 801

private slots:
    void CreateModel(QAstra* pqastra,CUIForm* pUIForm);
    void onRTDM_UpdateModel(std::string tname);
    void onRTDM_UpdateView(std::string tname);
    void onRTDM_BeginResetModel(std::string tname);


public:
    std::unique_ptr<RModel> prm;
    CUIForm m_UIForm;
    QAstra* m_pqastra;
    RTablesDataManager* m_pRTDM;
    ads::CDockManager* m_DockManager;

    //QTtitanGrid
    Qtitan::Grid* m_grid;
    Qtitan::GridTableView* view;
    LinkedForm m_lf;

private:
    using BufferRow = std::vector<QByteArray>;
    std::vector<std::vector<QByteArray>> m_buffer;
    QString m_generatorStamp;
    RTableView* ptv ;
    QSortFilterProxyModel *proxyModel;
    QModelIndex index;              // current index (Cell clicked)
    int form_indx;
    int column;                         // for header
    int row;
    Qtitan::GridTableColumn* column_qt;
    QPoint MenuRequestedPoint;
    std::string m_selection;                                                            // Текущая выборка
    std::map<int, std::vector<CondFormat>> m_MapcondFormatVector;                       // column , vector<CondFormat>
    QShortcut *sC_CTRL_I = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_I), this);
    QShortcut *sC_CTRL_D = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_D), this);
    QShortcut *sC_CTRL_R = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_R), this);
    QShortcut *sC_CTRL_A = new QShortcut( QKeySequence(Qt::CTRL | Qt::Key_A), this);
protected:
    itemStateMap tItemStateMap;
    QFrame customizeFrame;
    QListWidget customizeListWidget;
};

#endif // RTABWIDGET_H
