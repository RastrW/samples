#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <rtablesdatamanager.h>
#if(!defined(QICSGRID_NO))
    #include <QicsTable.h>
#endif //#if(!defined(QICSGRID_NO))

class QMdiArea;
class QSignalMapper;
class QHBoxLayout;

class QAstra;
class CUIForm;
class McrWnd;
struct _hint_data;
class rmodel;
namespace ads{ class CDockManager; }
class RtabWidget;
class FormProtocol;
enum MainTheme
{
    THEME_0 = -1,
    THEME_1 = Qt::darkRed,
    THEME_2 = Qt::darkMagenta,
    THEME_3 = Qt::darkGray,
    THEME_4 = Qt::darkGreen,
    THEME_5 = Qt::darkCyan,
};

enum StyleSetting
{
    DefaultStyleSetting = 0,
    Windows7ScenicStyleSetting,
    Office2016ColorfulStyleSetting,
    Office2016BDarkGrayStyleSetting,
    Office2016BlackStyleSetting,
    AdobePhotoshopLightGrayStyleSetting,
    AdobePhotoshopDarkGrayStyleSetting,
    VisualSudio2019BlueStyleSetting,
    VisualSudio2019DarkStyleSetting,
    FluentLightStyleSetting,
    FluentDarkStyleSetting
};

namespace spdlog{namespace level{enum level_enum;}}
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
    void file_loaded();                                                                     // загружен файл
    void rgm_signal();
   // void rm_change(std::string _t_name, QModelIndex index, QVariant value);
   // void rm_change(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void rm_RowInserted(std::string _t_name, int _row);
    void rm_RowDeleted(std::string _t_name, int _row);
    void rm_update(std::string _t_name);
    void signal_calc_begin();
    void signal_calc_end();
///slots.begin
private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void openRecentFile();
    void showFormSettings();
    void about();
    void rgm_wrap();
    void oc_wrap();
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
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;



private:
///slots.end.
public:
    MainWindow();
    virtual ~MainWindow();
    void setForms(const std::list<CUIForm>& forms); // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    void setQAstra(const std::shared_ptr<QAstra>& sp_qastra);
private:
    QHBoxLayout* createStyleSetting();
    void setCurrentFile(const QString &fileName, const std::string Shablon = "");
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    QString curFile;

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
    bool maybeSave();
#if(!defined(QICSGRID_NO))
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
#endif
    void closeEvent(QCloseEvent *event) override;
    QMdiArea*          m_workspace     = nullptr;
    McrWnd*            m_pMcrWnd       = nullptr;
    FormProtocol*      m_pFormProtocol = nullptr;
    QSignalMapper*     m_windowMapper  = nullptr;
    QMenu*             m_menuOpen      = nullptr;
    QMenu*             m_recentFilesMenu=nullptr;
    QMenu*             m_menuCalcParameters = nullptr;
    QAction     *separatorAct;
    QHBoxLayout*       m_layoutActions = nullptr;                 // actions: rgm,opf,...
    QToolBar*          m_toolbarCalc   = nullptr;
    RtabWidget*        m_prtw_current  = nullptr;                  // current table
    std::string        m_cur_file;                      // current file
    QDockWidget*       m_dock        = nullptr;
    ads::CDockManager* m_DockManager = nullptr; // The main container for docking
    _v_cache_log       m_v_cache_log;
    std::shared_ptr<QAstra> m_sp_qastra;

    //RTablesDataManager<QDenseDataBlock<FieldVariantData>> m_RTDM;
    RTablesDataManager m_RTDM;

    std::list<CUIForm> m_lstUIForms;
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
};
#endif // MAINWINDOW_H
