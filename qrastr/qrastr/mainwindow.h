#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QMessageBox>

#if(!defined(QICSGRID_NO))
    #include <QicsTable.h>
#endif //#if(!defined(QICSGRID_NO))

#include "astra_exp.h"
#include "License2/json.hpp"
#include "params.h"
#include "rastrhlp.h"

#include <QMenu>

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

    void showEvent( QShowEvent* event ) {
        try{
            QWidget::showEvent( event );
            //your code here
            // https://stackoverflow.com/questions/14161100/which-qt-widget-should-i-use-for-message-display
            int nRes = 0;
            QString str_curr_path = QDir::currentPath();
            std::string str_path_2_conf = "undef";
#if(defined(COMPILE_WIN))
            str_path_2_conf = R"(C:\projects\git_web\samples\qrastr\qrastr\appsettings.json)";
#else
            str_path_2_conf = R"(/home/ustas/projects/git_web/samples/qrastr/qrastr/appsettings.json)";
            //QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"), QString("In lin not implemented!") );      mb.exec();
#endif
            Params pars;
            nRes = pars.ReadJsonFile(str_path_2_conf);
            if(nRes<0){
                QMessageBox mb;
                // так лучше не делать ,смешение строк qt и std это боль.
                QString qstr = QObject::tr("Can't load on_start_file: ");
                std::string qstr_fmt = qstr.toUtf8().constData(); //  qstr.toStdString(); !!not worked!!
                std::string ss = fmt::format( "{}{} ", qstr_fmt.c_str(), pars.Get_on_start_load_file_rastr());
                QString str = QString::fromUtf8(ss.c_str());
                mb.setText(str);
                mb.exec();
                return ;
            }
            up_rastr_ = std::make_unique<CRastrHlp>();
            nRes = up_rastr_->CreateRastr();
            nRes = up_rastr_->Load(pars.Get_on_start_load_file_rastr());
            nRes = up_rastr_->ReadForms(pars.Get_on_start_load_file_forms());
            if(nRes<0){
                QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                QString("error: %1 wheh read file : %2").arg(nRes).arg(pars.Get_on_start_load_file_forms().c_str())
                               );
                mb.exec();
                return ;
            }
            setForms();
        }catch(const std::exception& ex){
            exclog(ex);
        }catch(...){
            exclog();
        }
    }

    void setForms(){ // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise

        int i = 0;

        QMap<QString,QMenu *> map_menu;
        //QMap<QString, QMenu>::iterator it ;
        auto forms = up_rastr_->GetForms();
        for(const auto& j_form : forms){
            std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
            if (!str_MenuPath.empty() && str_MenuPath.at(0) == '_')
                continue;
            QString qstr_MenuPath = str_MenuPath.c_str();
            if (!str_MenuPath.empty() && !map_menu.contains(qstr_MenuPath))
                map_menu.insert(qstr_MenuPath,m_openMenu->addMenu(str_MenuPath.c_str()));
        }
        //for(const nlohmann::json& j_form : j_forms_){
        for(const auto& j_form : forms){
            std::string str_Name = stringutils::cp1251ToUtf8(j_form.Name());
            std::string str_TableName = j_form.TableName();
            std::string str_MenuPath = stringutils::cp1251ToUtf8(j_form.MenuPath());
            QString qstr_MenuPath = str_MenuPath.c_str();
            QMenu* cur_menu = m_openMenu;
            if (map_menu.contains(qstr_MenuPath))
                cur_menu = map_menu[qstr_MenuPath];
            if (!str_Name.empty() && str_Name.at(0) != '_'){
                QAction* p_actn = cur_menu->addAction(str_Name.c_str());
                p_actn->setData(i);
            }
            i++;
        }

        connect( m_openMenu, SIGNAL(triggered(QAction *)),
                this, SLOT(onOpenForm(QAction *)), Qt::UniqueConnection);

/*
        //for(const nlohmann::json& j_form : j_forms_){
            //std::string str_Collection = j_form["Collection"];
            std::string str_MenuPath = j_form["MenuPath"];
            if (!str_MenuPath.empty() && str_MenuPath.at(0) == '_')
                continue;

            QString qstr_MenuPath = str_MenuPath.c_str();
            if (!str_MenuPath.empty() && !map_menu.contains(qstr_MenuPath))
                map_menu.insert(qstr_MenuPath,m_openMenu->addMenu(str_MenuPath.c_str()));
        }

        for(const nlohmann::json& j_form : j_forms_){
            std::string str_Name = j_form["Name"];
            std::string str_TableName = j_form["TableName"];
            std::string str_MenuPath = j_form["MenuPath"];
            QString qstr_MenuPath = str_MenuPath.c_str();
            QMenu* cur_menu = m_openMenu;
            if (map_menu.contains(qstr_MenuPath))
                cur_menu = map_menu[qstr_MenuPath];
            if (!str_Name.empty() && str_Name.at(0) != '_')
            {
                QAction* p_actn = cur_menu->addAction(str_Name.c_str());
                p_actn->setData(i);
            }
            i++;
        }

        connect( m_openMenu, SIGNAL(triggered(QAction *)),
                this, SLOT(onOpenForm(QAction *)), Qt::UniqueConnection);
*/
    }

    //void SetIdrastr(_idRastr id_rastr_in);
    //void setForms(nlohmann::json& j_forms_in);

#if(!defined(QICSGRID_NO))
    // Returns pointer to the table that is active, otherwise returns NULL
    QicsTable* activeTable();
#endif//#if(!defined(QICSGRID_NO))

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
    void insertRow();
    void deleteRow();
    void insertCol();
    void deleteCol();
    void about();
    void rgm_wrap();
    void onOpenForm(QAction* p_actn);
#if(!defined(QICSGRID_NO))
    void sortAscending();
    void sortDescending();
#endif//#if(!defined(QICSGRID_NO))
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

    QAction *m_RGMAct;

    nlohmann::json             j_forms_;
    _idRastr                   id_rastr_ = -1;
    std::unique_ptr<CRastrHlp> up_rastr_;
public:
    QAction *m_SortAscAct;
    QAction *m_SortDescAct;
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
