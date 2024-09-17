#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QMessageBox>
#include <QDockWidget>
#include <QTableView>
#include <QMenu>

#if(!defined(QICSGRID_NO))
    #include <QicsTable.h>
#endif //#if(!defined(QICSGRID_NO))
#include "common_qrastr.h"
#include "License2/json.hpp"
#include "params.h"
#include "rastrhlp.h"
#include "rmodel.h"
#include "rtabwidget.h"

class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class MdiChild;
class QSignalMapper;

class QAstra ;
class CUIForm;
struct _hint_data;
class rmodel;
namespace ads{ class CDockManager; }
class MainWindow
    : public QMainWindow{
    Q_OBJECT
public:
    struct _cache_log{
        spdlog::level::level_enum lev;
        std::string               str_log;
        _cache_log( const spdlog::level::level_enum lev_in, std::string_view sv_in );
        _cache_log& operator=(const _cache_log&  cache_log);
        _cache_log& operator=(const _cache_log&& cache_log);
        _cache_log           (const _cache_log&  cache_log);
        _cache_log           (const _cache_log&& cache_log);
    };
    struct _v_cache_log
        : public std::vector<_cache_log> {
        template <typename... Args>
        void add( const spdlog::level::level_enum lev_in, const std::string_view sv_format, Args&&... args );
    };
signals:
    void file_loaded(CRastrHlp& _rh);                                                                     // загружен файл
    void rgm_signal();
    void rm_change(std::string _t_name, QModelIndex index, QVariant value);
    void rm_change(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void rm_RowInserted(std::string _t_name, int _row);
    void rm_RowDeleted(std::string _t_name, int _row);
    void rm_update(std::string _t_name);
///slots.begin
private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    void insertRow();
    void deleteRow();
    void insertCol();
    void deleteCol();
    void about();
    void rgm_wrap();
    void onDlgMcr();
    void onOpenForm(QAction* p_actn);
    void onButton2Click();
    void Btn1_onClick();
    void Btn3_onClick();
    void ondataChanged(std::string _t_name, QModelIndex index, QVariant value);
    void ondataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void onRowInserted(std::string _t_name, int _row);
    void onRowDeleted(std::string _t_name, int _row);
#if(!defined(QICSGRID_NO))
    void sortAscending();
    void sortDescending();
#endif//#if(!defined(QICSGRID_NO))
    void onItemPressed(const QModelIndex &index);
public slots:
    void updateMenus();
    void updateWindowMenu();
#if(!defined(QICSGRID_NO))
    MdiChild *createMdiChild(nlohmann::json j_form = "");
#endif//#if(!defined(QICSGRID_NO))
    void setActiveSubWindow(QWidget *window);
    void tst_onRastrHint(const _hint_data&);
private:
///slots.end.
public:
    MainWindow();
    virtual ~MainWindow();
    long init();
    void setForms(std::list<CUIForm>& forms); // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    void setQAstra(std::shared_ptr<QAstra> sp_qastra);
private:
    int  readSettings();
    int  writeSettings();
    void showEvent( QShowEvent* event ) override;
#if(!defined(QICSGRID_NO))
    QicsTable* activeTable(); // Returns pointer to the table that is active, otherwise returns NULL
#endif//#if(!defined(QICSGRID_NO))
    void createActions();
    void createMenus();
    void createToolBars();
    void loadPlugins();
    void createCalcLayout();
    void createStatusBar();
    void logCacheFlush();
#if(!defined(QICSGRID_NO))
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
#endif
    void closeEvent(QCloseEvent *event)override;
    QMdiArea* m_workspace;
    QSignalMapper *m_windowMapper;
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_CalcMenu;
    QMenu *m_openMenu;
    QMenu *m_windowMenu;
    QMenu *m_helpMenu;
    QHBoxLayout *m_ActionsLayout;   // actions: rgm,opf,...
    QToolBar *m_fileToolBar;
    QToolBar *m_editToolBar;
    QToolBar *m_viewToolBar;
    QToolBar *m_calcToolBar;
    QAction *m_newAct;
    QAction *m_openAct;
    QAction *m_saveAct;
    QAction *m_saveAsAct;
    QAction *m_exitAct;
    QAction *m_cutAct;
    QAction *m_copyAct;
    QAction *m_pasteAct;
    QAction *m_insertRowAct;
    QAction *m_deleteRowAct;
    QAction *m_insertColAct;
    QAction *m_deleteColAct;
    QAction *m_closeAct;
    QAction *m_closeAllAct;
    QAction *m_tileAct;
    QAction *m_cascadeAct;
    QAction *m_arrangeAct;
    QAction *m_nextAct;
    QAction *m_previousAct;
    QAction *m_separatorAct;
    QAction *m_aboutAct;
    //QAction *m_SortAscAct;
    //QAction *m_SortDescAct;
    QAction* m_RGMAct;
    QAction* m_ActMacro;
    QAction* m_TestAct;
    nlohmann::json             j_forms_;
    _idRastr                   id_rastr_ = -1;
    std::unique_ptr<CRastrHlp> up_rastr_;
    RtabWidget *prtw_current;                        // current table
    std::string cur_file;                            // current file
    QAction *m_SortAscAct;
    QAction *m_SortDescAct;
    QDockWidget *m_dock;
    ads::CDockManager* m_DockManager = nullptr; // The main container for docking
    rmodel *model;
    Params m_params;
    QDir qdirData_;
    _v_cache_log v_cache_log_;
    std::shared_ptr<QAstra> m_sp_qastra;
    std::list<CUIForm> m_lstUIForms;

    static constexpr char pchSettingsDirData_[5]{"Data"};
    static constexpr char pchSettingsOrg_[7]{"QRastr"};

};
#endif // MAINWINDOW_H
