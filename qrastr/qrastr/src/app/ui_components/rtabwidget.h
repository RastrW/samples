#pragma once

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
#include "rtableview.h"
#include "rtablesdatamanager.h"

#include "linkedform.h"

namespace ads{ class CDockManager; }

class RtabWidget;
class QMimeData;
class QAstra;
class LinkedForm;

/**
 * @brief
    - Отображение табличных данных в QTitan Grid
    - Управление взаимодействием пользователя с таблицей
    - Обработка контекстных меню
    - Связывание с RModel
    - Поддержка условного форматирования
    - Управление связанными формами
 */
class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RtabWidget(CUIForm UIForm,QWidget *parent = nullptr);
    explicit RtabWidget(QAstra* pqastra, CUIForm UIForm,
                        RTablesDataManager* pRTDM, ads::CDockManager* pDockManager ,QWidget *parent = nullptr);
    virtual ~RtabWidget() = default;

    void closeEvent(QCloseEvent* event) override;
private:  
    void setTableView(QTableView& tv, RModel& mm, int myltiplier = 10);
    void setTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier = 10 );

    std::tuple<int,double> GetSumSelected();
    QMenu* CunstructLinkedFormsMenu(std::string form_name);

signals:
    void CondFormatsModified();
public slots:
    void on_calc_begin();
    void on_calc_end();
    void OnClose();
    void contextMenu(ContextMenuEventArgs* args);

    void onItemPressed(CellClickEventArgs* _index);
    void onfocusRowChanged( int _row_old,int _row_new);

    void AddRow();
    void insertRow_qtitan();
    void DuplicateRow_qtitan();
    void deleteRow_qtitan();
    // ширина по шаблону
    void widebyshabl();
    // ширина по контенту
    void widebydata();
    void OpenColPropForm();
    void OpenSelectionForm();

    void OpenGroupCorrection();
    void OpenExportCSVForm();
    void OpenImportCSVForm();

    void SetSelection(std::string Selection);
    void SetDirectCodeEntry(size_t column);
    void editCondFormats(size_t column);
    void onCondFormatsModified();
    void SetLinkedForm( LinkedForm _lf);
    void onOpenLinkedForm(LinkedForm _lf );    // ТИ:Каналы ; id1=%d & id2=0 & prv_num<8 ; 801
private slots:
    void CreateModel(QAstra* pqastra,CUIForm* pUIForm);
    void SetEditors();
    void SetEditor(RCol& _prcol);

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
    RTableView* ptv;
    int column;                         // for header
    int row;
    Qtitan::GridTableColumn* column_qt;
    std::string m_selection;                                                            // Текущая выборка
    std::map<int, std::vector<CondFormat>> m_MapcondFormatVector;                       // column , vector<CondFormat>
};
