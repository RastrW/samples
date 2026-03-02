#pragma once
#include <QWidget>
#include "UIForms.h"
#include "CondFormat.h"
#include "QtitanGrid.h"

namespace ads{ class CDockManager; }

class RtabWidget;
class QMimeData;
class QAstra;
class PyHlp;
class LinkedFormController;
class LinkedForm;
class QToolBar;
class RTablesDataManager;
class RModel;
class RCol;
class QTableView;
class RTableView;
class ContextMenuBuilder;

///@brief Виджет, отображающий одну таблицу Rastr в QTitan Grid.
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
                        ads::CDockManager* pDockManager,
                        QWidget *parent = nullptr);
    virtual ~RtabWidget() = default;
    //отключает сигналы LinkedForm, сбрасывает pnparray_.
    void closeEvent(QCloseEvent* event) override;
    QWidget* createDockContent(bool addToolbar = true);

    int getLongValue(const std::string& key, long row);
    /// @brief Применяет LinkedForm через контроллер.
    void applyLinkedFormFromController(const LinkedForm& lf);
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

    void slot_addRow();
    void slot_insertRow();
    void slot_duplicateRow();
    void slot_deleteRow();
    void slot_groupCorrection();
    // ширина по шаблону
    void slot_widthByTemplate();
    // ширина по контенту
    void slot_widthByData();

    //  Формы инструментов
    void slot_openColProp();
    void slot_openSelection();

    void slot_openExportCSVForm();
    void slot_openImportCSVForm();

    void slot_directCodeToggle(std::size_t column);
    void slot_condFormatsEdit(std::size_t column);

    void SetSelection(std::string Selection);
    void onCondFormatsModified();

    void slot_beginResetModel(std::string tname);
    void slot_endResetModel(std::string tname);
private slots:

    void applyAllColumnEditors ();
    void applyColumnEditor(int colIndex);
private:
    void setTableView(QTableView& tv, RModel& mm, int multiplier = 10);
    void setTableView(Qtitan::GridTableView& tv, RModel& mm,
                      int multiplier = 10 );
    /// Блокирует горячие клавиши заданные по умолчанию в Qtitan
    bool eventFilter(QObject* obj, QEvent* event) override;  // ← п.8
    /** @brief
     * a) создаёт RModel, вызывает setForm/populateDataFromRastr;
     * b) подключает сигналы RTDM к слотам RModel (обновления данных);
     * c) устанавливает редакторы колонок (SetEditors);
     * d) восстанавливает условное форматирование из JSON.
    */
    void createModel();

    void setupToolbar();
    void setupShortcuts();

    std::tuple<int,double> GetSumSelected();

    Qtitan::Grid* m_grid;
    Qtitan::GridTableView* m_view;
    std::shared_ptr<PyHlp> pPyHlp_;
    std::unique_ptr<RModel> m_model;
    std::unique_ptr<LinkedFormController> m_linkedFormCtrl;
    std::unique_ptr<ContextMenuBuilder>   m_menuBuilder;
    CUIForm m_UIForm;
    QAstra* m_pqastra;
    RTablesDataManager* m_pRTDM;
    ads::CDockManager* m_DockManager;

    QToolBar* m_toolbar;
    // Действия в toolbar
    QAction* m_actAddRow;
    QAction* m_actInsertRow;
    QAction* m_actDeleteRow;
    QAction* m_actDuplicateRow;
    QAction* m_groupCorrection;

    RTableView* ptv;
    // Колонка и строка, зафиксированные в момент открытия контекстного меню.
    int m_contextMenuColumn;
    int m_contextMenuRow;
    Qtitan::GridTableColumn* column_qt;
    std::string m_selection {""}; // Текущая выборка
    std::map<int, std::vector<CondFormat>>
        m_MapcondFormatVector; // column , vector<CondFormat>
    std::map<QString,bool> m_ColumnsVisible;
};
