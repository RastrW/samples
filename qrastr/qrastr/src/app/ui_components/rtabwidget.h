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
class PyHlp;

/**
 * @brief Виджет, отображающий одну таблицу Rastr в QTitan Grid.
 *
 * Жизненный цикл:
 *   1. FormManager создаёт RtabWidget, передавая QAstra*, CUIForm, RTDM*, DockManager*.


 *   4. closeEvent(): отключает сигналы LinkedForm, сбрасывает pnparray_.
 *
 * Связанные формы (LinkedForm):
 *   onOpenLinkedForm() создаёт новый RtabWidget в нижней панели Dock
 *   и подписывает его на события фокусирования строки родительской формы.
 */
class RtabWidget : public QWidget
{
    Q_OBJECT
public:
    /** @brief
     *  Конструктор:
     *      a) создаёт QTitan Grid и настраивает опции отображения;
     *      b) привязывает горячие клавиши (Ctrl+I/A/R/D);
     *      c) вызывает CreateModel() — строит RModel и заполняет данные.
    */
    explicit RtabWidget(QAstra* pqastra, CUIForm UIForm,
                        RTablesDataManager* pRTDM,
                        ads::CDockManager* pDockManager ,QWidget *parent = nullptr);
    virtual ~RtabWidget() = default;

    void closeEvent(QCloseEvent* event) override;
    QWidget* createDockContent();
private:  
    void setTableView(QTableView& tv, RModel& mm, int myltiplier = 10);
    void setTableView(Qtitan::GridTableView& tv, RModel& mm, int myltiplier = 10 );

    void setupToolbar();

    std::tuple<int,double> GetSumSelected();
    QMenu* CunstructLinkedFormsMenu(std::string form_name);
    QMenu* CunstructLinkedMacroMenu(std::string form_name);

signals:
    void CondFormatsModified();
public slots:
    void on_calc_begin();
    void on_calc_end();
    void OnClose();
    void contextMenu(ContextMenuEventArgs* args);
    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);

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
    void SetDirectCodeEntry(std::size_t column);
    void editCondFormats(std::size_t column);
    void onCondFormatsModified();
    void SetLinkedForm( LinkedForm _lf);
    void onOpenLinkedForm(LinkedForm _lf );    // ТИ:Каналы ; id1=%d & id2=0 & prv_num<8 ; 801
    void slot_beginResetModel(std::string tname);
    void slot_endResetModel(std::string tname);
    void onOpenLinkedMacro(LinkedMacro _lm );

private slots:
    /** @brief
     * a) создаёт RModel, вызывает setForm/populateDataFromRastr;
     * b) подключает сигналы RTDM к слотам RModel (обновления данных);
     * c) устанавливает редакторы колонок (SetEditors);
     * d) восстанавливает условное форматирование из JSON.
    */
    void CreateModel(QAstra* pqastra,CUIForm* pUIForm);
    void SetEditors();
    void SetEditor(RCol& _prcol);
public:
    std::unique_ptr<RModel> prm;
    CUIForm m_UIForm;
    QAstra* m_pqastra;
    RTablesDataManager* m_pRTDM;
    ads::CDockManager* m_DockManager;
private:
    Qtitan::Grid* m_grid;
    Qtitan::GridTableView* view;
    LinkedForm m_lf;
    std::shared_ptr<PyHlp> pPyHlp_;

    QToolBar* m_toolbar;

    // Действия в toolbar
    QAction* m_actAddRow;
    QAction* m_actInsertRow;
    QAction* m_actDeleteRow;
    QAction* m_actDuplicateRow;

    RTableView* ptv;
    int column;                         // for header
    int row;
    Qtitan::GridTableColumn* column_qt;
    std::string m_selection;                                      // Текущая выборка
    std::map<int, std::vector<CondFormat>> m_MapcondFormatVector; // column , vector<CondFormat>
    std::map<QString,bool> m_ColumnsVisible;

};
