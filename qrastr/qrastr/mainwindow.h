#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QMessageBox>
#include <QDockWidget>

#if(!defined(QICSGRID_NO))
    #include <QicsTable.h>
#endif //#if(!defined(QICSGRID_NO))


#include "License2/json.hpp"
#include "params.h"
#include "rastrhlp.h"

#include <QTableView>
#include "rmodel.h"
#include "rtabwidget.h"

#include <QMenu>

class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class MdiChild;
class QSignalMapper;

class rmodel;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    QWidget* p_;
    void showEvent( QShowEvent* event ) override;
    void setForms(); // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    //void SetIdrastr(_idRastr id_rastr_in);
    //void setForms(nlohmann::json& j_forms_in);

#if(!defined(QICSGRID_NO))
    // Returns pointer to the table that is active, otherwise returns NULL
    QicsTable* activeTable();
#endif//#if(!defined(QICSGRID_NO))

signals:
    void file_loaded(CRastrHlp& _rh);                                                                     // загружен файл

    void rgm_signal();

    void rm_change(std::string _t_name, QModelIndex index, QVariant value);
    void rm_change(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void rm_RowInserted(std::string _t_name, int _row);
    void rm_RowDeleted(std::string _t_name, int _row);
    void rm_update(std::string _t_name);
protected:
    void closeEvent(QCloseEvent *event);

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

    void ondataChanged(std::string _t_name, QModelIndex index, QVariant value);
    void ondataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void onRowInserted(std::string _t_name, int _row);
    void onRowDeleted(std::string _t_name, int _row);

    //void SetTableView(QTableView& tv, RModel& mm);
#if(!defined(QICSGRID_NO))
    void sortAscending();
    void sortDescending();
#endif//#if(!defined(QICSGRID_NO))
    void onItemPressed(const QModelIndex &index);
public slots:

///slots.end.

    void updateMenus();
    void updateWindowMenu();

#if(!defined(QICSGRID_NO))
    MdiChild *createMdiChild(nlohmann::json j_form = "");
#endif//#if(!defined(QICSGRID_NO))

    void setActiveSubWindow(QWidget *window);
private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createCalcLayout();
    void createStatusBar();
    void readSettings();
    void writeSettings();
#if(!defined(QICSGRID_NO))
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
#endif
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
    nlohmann::json             j_forms_;
    _idRastr                   id_rastr_ = -1;
    std::unique_ptr<CRastrHlp> up_rastr_;
    RtabWidget *prtw_current;                        // current table
    std::string cur_file;                            // current file


public:
    QAction *m_SortAscAct;
    QAction *m_SortDescAct;
    QDockWidget *m_dock;
    //QTableView *table;
    rmodel *model;

};


/*
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
*/
#endif // MAINWINDOW_H
