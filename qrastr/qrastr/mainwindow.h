#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QicsTable.h>
#include "astra_exp.h"
#include "License2/json.hpp"

class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class MdiChild;
class QSignalMapper;


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    void SetIdrastr(_idRastr id_rastr_in);
    void setForms(nlohmann::json& j_forms_in);

    // Returns pointer to the table that is active, otherwise returns NULL
    QicsTable* activeTable();
signals:
    void rgm_signal();
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void cut();
    void copy();
    void paste();
    void about();
    void rgm_wrap();
    void sortAscending();
    void sortDescending();
    void onOpenForm(QAction* p_actn);
///slots.end.
    void updateMenus();
    void updateWindowMenu();
    MdiChild *createMdiChild(nlohmann::json j_form = "");
    void setActiveSubWindow(QWidget *window);
private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createCalcLayout();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
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
    QAction *m_closeAct;
    QAction *m_closeAllAct;
    QAction *m_tileAct;
    QAction *m_cascadeAct;
    QAction *m_arrangeAct;
    QAction *m_nextAct;
    QAction *m_previousAct;
    QAction *m_separatorAct;
    QAction *m_aboutAct;

    QAction *m_SortAscAct;
    QAction *m_SortDescAct;
    QAction *m_RGMAct;

    nlohmann::json j_forms_;
    _idRastr       id_rastr_;
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
