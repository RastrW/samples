#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMdiArea>
#include <QSignalMapper>

#if(!defined(QICSGRID_NO))
    #include <QicsTable.h>
#endif //#if(!defined(QICSGRID_NO))
#include "common_qrastr.h"
#include "License2/json.hpp"
#include "params.h"
#include "rastrhlp.h"
#include "rmodel.h"
#include "rtabwidget.h"
/*
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class MdiChild;
class QSignalMapper;*/

class QAstra;
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
    void onRowDeleted (std::string _t_name, int _row);
#if(!defined(QICSGRID_NO))
    void sortAscending();
    void sortDescending();
#endif//#if(!defined(QICSGRID_NO))
    void onItemPressed(const QModelIndex &index);
public slots:
    void updateMenus();
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
    void setForms(const std::list<CUIForm>& forms); // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    void setQAstra(const std::shared_ptr<QAstra>& sp_qastra);
private:
    int  readSettings();
    int  writeSettings();
    void showEvent( QShowEvent* event ) override;
#if(!defined(QICSGRID_NO))
    QicsTable* activeTable(); // Returns pointer to the table that is active, otherwise returns NULL
#endif//#if(!defined(QICSGRID_NO))
    void createActions();
    void createCalcLayout();
    void createStatusBar();
    void logCacheFlush();
#if(!defined(QICSGRID_NO))
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
#endif
    void closeEvent(QCloseEvent *event)override;
    QMdiArea*          m_workspace;
    QSignalMapper*     m_windowMapper;
    QMenu*             m_menuOpen;
    QHBoxLayout*       m_layoutActions;                 // actions: rgm,opf,...
    QToolBar*          m_toolbarCalc;
    RtabWidget*        m_prtw_current;                  // current table
    std::string        m_cur_file;                      // current file
    QDockWidget*       m_dock        = nullptr;
    ads::CDockManager* m_DockManager = nullptr; // The main container for docking
    Params             m_params;
    _v_cache_log       m_v_cache_log;
    std::shared_ptr<QAstra> m_sp_qastra;
    std::list<CUIForm> m_lstUIForms;

    static constexpr char m_pchSettingsDirData[] {"Data"};
    static constexpr char m_pchSettingsOrg[]     {"QRastr"};
};
#endif // MAINWINDOW_H
