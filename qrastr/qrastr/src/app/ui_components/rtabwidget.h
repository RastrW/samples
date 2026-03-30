#pragma once
#include <QWidget>
#include "UIForms.h"
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
class ContextMenuBuilder;
class CondFormatController;

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
    virtual ~RtabWidget();
    //отключает сигналы LinkedForm, сбрасывает pnparray_.
    void closeEvent(QCloseEvent* event) override;
    QWidget* createDockContent(bool addToolbar = true);

    int getLongValue(const std::string& key, long row);
    /// @brief Применяет LinkedForm через контроллер.
    void applyLinkedFormFromController(const LinkedForm& lf);
    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);
    void on_calc_begin();
    void on_calc_end();
public slots:

    void slot_close();
    void slot_contextMenu(ContextMenuEventArgs* args);

    void slot_focusRowChanged( int _row_old,int _row_new);

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
    void slot_openColProp(int col);
    void slot_openSelection();

    void slot_openExportCSVForm();
    void slot_openImportCSVForm();

    void slot_directCodeToggle(std::size_t column);
    void slot_condFormatsEdit(std::size_t column);

    void slot_setFiltrForSelection(std::string selection);

    void slot_beginResetModel(std::string tname);
    void slot_endResetModel(std::string tname);
private slots:

    void applyAllColumnEditors ();
    void applyColumnEditor(int colIndex);
private:
    /** @brief
     * a) создаёт RModel, вызывает setForm/populateDataFromRastr;
     * b) подключает сигналы RTDM к слотам RModel (обновления данных);
     * c) устанавливает редакторы колонок (SetEditors);
     * d) восстанавливает условное форматирование из JSON.
    */
    void createModel();

    void setTableView(int multiplier = 10 );
    /// Блокирует горячие клавиши заданные по умолчанию в Qtitan
    bool eventFilter(QObject* obj, QEvent* event) override;

    void setupToolbar();
    void setupShortcuts();
    void setupConnections();

    Qtitan::Grid* m_grid;
    Qtitan::GridTableView* m_view;
    std::shared_ptr<PyHlp> pPyHlp_;

    // ── Компоненты ──
    std::unique_ptr<RModel> m_model;
    std::unique_ptr<LinkedFormController> m_linkedFormCtrl;
    std::unique_ptr<ContextMenuBuilder>   m_menuBuilder;
    std::unique_ptr<CondFormatController> m_condFormatCtrl;

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

    std::string m_selection {""}; // Текущая выборка
    std::unordered_map<QString,bool>
        m_columnsVisible;
};
