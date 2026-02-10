#include <QSplitter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <QCloseEvent>
#include <QQmlDebuggingEnabler>
QQmlDebuggingEnabler enabler;

#include "mcrwnd.h"
#include "scihlp.h"
#include "tst_toolbox.h"
#include "tst2_dialog.h"
#include "forms/dlgfindrepl.h"
//#include "IPlainRastr.h"
#include "../app/astra/qastra_events_data.h"
#include "pyhlp.h"


const std::string g_str_tst_rastr_events {
R"(
import time
import struct
import socket
import sys

file_test = "../Data/Files/cx195.rg2"
file_test_templ = "../Data/SHABLON/режим.rg2"

#import astra_py as astra
rastr2 = astra.Rastr()

rastr.load(astra.LoadCode.REPL, file_test, file_test_templ)
rastr.print(f"Загружен файл {file_test} по шаблону {file_test_templ}")

try:
  ret = rastr.rgm("p")
  rastr.print('Run rgm(p): retcode = ', str(ret) )
except Exception  as err:
  rastr.print('Handling run-time error:', err)

rastr.print( f"")
for i in range(0,10,1):
    if (i == 3): continue
    if (i == 6): break
    rastr.print( f"{i} : итерация цикла")

node=rastr.table("node")
pn= node.column("pn")
node.set_selection("na=1")
pn.calculate("pn*1.2") # изменяем нагрузку в районе 1
rastr.rgm("zcr") # повторный расчет режима

kod = rastr.rgm("")
if (kod != astra.ASTCode.OK):
    rastr.print("Режим не сбалансирован")

#Запись в файл номеров и названий линий. Разделитель – точка с запятой.
file = "c:/tmp/a.csv"
vetv = rastr["vetv"]
vetv.set_selection("x>100") # по условию
#vetv.set_selection("sta")  # отключенных линий
#!lin not work! vetv.to_csv(astra.CSVCode.REPLACE, file, "ip,iq,name", ";")

#доступ к данным
#rastr.print( f"")
#tnode = rastr["node"]
#ss = tnode.set_selection("=1,2,3")
#data = tnode.data(["ny", "name", "uhom"])
#rastr.print(data["ny"])
#rastr.print(data["name"])
#rastr.print(data["uhom"])
#rastr.print(data["index"])

#Пересчет реактивного сопротивления ЛЭП пропорционально активному.
tvetv = rastr["vetv"]
tvetv.set_selection("tip=0")
tvetv["x"].calculate("4*r")

#Перечисление колонок таблицы
rastr.print( f"")
number = 0
for col in rastr["node"].columns():
    rastr.print(col.name)
    number+=1
    if number == 3 :    # если number = 3, выходим из цикла
        break

#Перечисление таблиц
rastr.print( f"")
tabs = rastr.tables()
for i in range(0, tabs.count-1):
    rastr.print(tabs[i])
    if (i == 6): break

import tkinter as tk
from tkinter import ttk

root = tk.Tk()
root.title("Widgets Demo")

widgets = [
    tk.Label,
    tk.Checkbutton,
    ttk.Combobox,
    tk.Entry,
    tk.Button,
    tk.Radiobutton,
    tk.Scale,
    tk.Spinbox,
]

for widget in widgets:
    try:
        widget = widget(root, text=widget.__name__)
    except tk.TclError:
        widget = widget(root)
    widget.pack(padx=5, pady=5, fill="x")

root.mainloop()

from turtle import Turtle
from random import random

t = Turtle()
for i in range(30):
    steps = int(random() * 100)
    angle = int(random() * 360)
    t.right(angle)
    t.fd(steps)
t.screen.mainloop()


#from tkinter import *
#root = Tk()
#root.geometry('160x140+800+300')
#Label(text='Thats all folks', width=20, height=3).pack()
#root.mainloop()



)"};

const std::string g_str_tst_rastr_prot {
R"(
  <!DOCTYPE html>
  <html>
  <body>

  <h1>My First Heading</h1>
  <p>My first paragraph.</p>

  </body>
  </html>

  <!DOCTYPE html>
  <html lang="ru">
  <!---see https://medium.com/@gottfrois/temporary-queues-with-rabbitmq-using-web-stomp-e98e266d61e1
  sokjs-->
    <head>
      <meta http-equiv="content-type" content="text/html; charset=utf-8">
      <meta http-equiv="X-UA-Compatible" content="IE=edge">
      <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
      <meta name="description" content="">
      <meta name="author" content="">
      <title> RastrW admin </title>
      <!-- Custom fonts for this template-->
      <link href="vendor/fontawesome-free/css/all.min.css" rel="stylesheet" type="text/css">
    <!--  <link href="https://fonts.googleapis.com/css?family=Nunito:200,200i,300,300i,400,400i,600,600i,700,700i,800,800i,900,900i" rel="stylesheet"> -->
      <!-- Custom styles for this template-->
      <link href="css/sb-admin-2.min.css" rel="stylesheet">
      <link href="css/style.css" rel="stylesheet">

        <!-- Custom styles for this page -->
      <link href="vendor/datatables/dataTables.bootstrap4.min.css" rel="stylesheet"> <!-- стрелочки сортировки в таблице -->

  <!-- <script src="https://d3js.org/d3.v4.js"></script> -->
  <!--     <script src="vendor/d3js/d3.v4.js"></script> -->
  <script src="vendor/d3js/d3.v7.js"></script>
  <!-- <script src="vendor/d3js/d3.v7.js"></script> -->

    </head>

    <body id="page-top" style="background-color: white;">
      <!-- Page Wrapper -->
      <div id="wrapper">
        <!--  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  -->
        <!-- Sidebar -->
        <ul class="navbar-nav bg-gradient-primary sidebar sidebar-dark accordion" id="accordionSidebar">
          <!-- Sidebar - Brand -->
              <a class="sidebar-brand d-flex align-items-center justify-content-center"  href="index.html">
              <img title="RastrW.3" src="./img/RastrWin3_32x32.ico" />
          <div class="sidebar-brand-text mx-3"> Rastr.W <sup>3</sup></div>
          </a>
          <!-- Divider -->
          <hr class="sidebar-divider my-0">
          <!-- Divider -->
          <hr class="sidebar-divider">

          <!-- Divider -->
          <hr class="sidebar-divider d-none d-md-block">
          <!-- Sidebar Toggler (Sidebar) -->
          <div class="text-center d-none d-md-inline"> <button class="rounded-circle border-0" id="sidebarToggle"></button> </div>
        </ul>
        <!-- End of Sidebar -->

        <!--  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  -->

        <!-- Content Wrapper -->
        <div id="content-wrapper" class="d-flex flex-column">
          <!-- Main Content -->
          <div id="content">
            <!-- Topbar -->
            <nav class="navbar navbar-expand navbar-light bg-white topbar mb-4 static-top shadow">
              <!-- Sidebar Toggle (Topbar) -->
                    <!-- <button id="sidebarToggleTop" class="btn btn-link d-md-none rounded-circle mr-3"> WWWWW        </button> -->
                    <!-- Topbar Search -->
              <ul class="navbar-nav ml-auto">
                <li class="nav-item dropdown no-arrow mx-1"><a class="nav-link dropdown-toggle" href="#" id="alertsDropdown" role="button" data-toggle="dropdown"  aria-haspopup="true" aria-expanded="false"> </a>
                <!-- Nav Item - User Information -->
                <li class="nav-item dropdown no-arrow">
                  <a class="nav-link dropdown-toggle" href="#" id="userDropdown" role="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                  <span class="mr-2 d-none d-lg-inline text-gray-600 small"> Douglas McGee </span>
                  <img class="img-profile rounded-circle" src="img/undraw_profile.svg">
                  </a>
                  <!-- Dropdown - User Information -->
                  <div class="dropdown-menu dropdown-menu-right shadow animated--grow-in" aria-labelledby="userDropdown">
                          <a class="dropdown-item" href="#"> <i class="fas fa-user fa-sm fa-fw mr-2 text-gray-400"></i> Profile </a>
                          <a class="dropdown-item" href="#"> <i class="fas fa-cogs fa-sm fa-fw mr-2 text-gray-400"></i> Settings </a>
                          <!-- <a class="dropdown-item" href="#"> <i class="fas fa-list fa-sm fa-fw mr-2 text-gray-400"></i> Activity Log </a> -->
                  <!-- <div class="dropdown-divider"></div> -->
                          <a class="dropdown-item" href="#" data-toggle="modal" data-target="#logoutModal"> <i class="fas fa-sign-out-alt fa-sm fa-fw mr-2 text-gray-400"></i> Logout </a>
                          <a class="dropdown-item" href="#" data-toggle="modal" data-target="#loginModal">  <i class="fas fa-sign-in-alt fa-sm fa-fw mr-2 text-gray-400"></i> LogIn  </a>
                        </div>
                </li>
              </ul>
            </nav>
            <!-- End of Topbar -->

            <!--  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  -->

            <!-- Begin Page Content -->
            <div class="container-fluid">
              <!-- Page Heading -->
              <div class="d-sm-flex align-items-center justify-content-between mb-4">

               <div>
                <div class="container">


                    BUTTON
                  </button>
                </div>
              </div>
           </div>

           <!--!!!!!!!!!!!!!!!!! SVG for d3 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!-->
          <!--
  for d3VG for d3VG for d3VG for d3VG for d3VG for d3for d3VG for d3VG for d3VG for d3VG for d3VG for d3

  for d3VG for d3VG for d3VG for d3VG for d3VG for d3
  !!!!!!!!!!!!!!!! SVG for d3VG for d3VG for d3VG for d3VG for d3VG for d3 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!-->
    </body>
  </html>
)"};

McrWnd::McrWnd(QWidget* parent, const _en_role en_role)
    : QDialog(parent,
              Qt::WindowMinimizeButtonHint |
              Qt::WindowMaximizeButtonHint |
              Qt::WindowCloseButtonHint
     ),en_role_(en_role){
    const int nWidth = 600;
    const int nHeight = 800;
    resize(nWidth, nHeight);
    setWindowIcon( QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon) ));
    setWindowTitle(tr("Macro Python"));
    QSplitter* splitter = new QSplitter(this);
    QVBoxLayout* layout = new QVBoxLayout();
    QVBoxLayout* container_layout = new QVBoxLayout();
    QLineEdit* leFind = new QLineEdit();    leFind->setFixedWidth(100);
    if(_en_role::macro_dlg == en_role_){
        QToolBar* pToolBar = new QToolBar();
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)),         tr("&New"),     this,  SLOT( onFileNew() )    )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_N)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon)),          tr("&Open"),    this,  SLOT( onFileOpen() )   )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_O)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton)), tr("&Save"),    [this] { onFileSave(false); } )
                ->setShortcuts( {QKeySequence(Qt::CTRL+Qt::Key_S)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)),      tr("Save as"),  [this] { onFileSave(true); }  )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_W)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)),        tr("Run (F10)"), this, SLOT( onRun() )        )
                ->setShortcut({QKeySequence(Qt::Key_F10)});
        //pToolBar->addWidget(leFind);
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)),    tr("&Find"),     this, SLOT( onFind() )       )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_F)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink)),      tr("&Go to line"), this, SLOT( onGoToLine() ) )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_G)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogResetButton)), tr("&Clear"), this, SLOT( onProtClear() ) );


        container_layout->addWidget(pToolBar);
        shEdit_ = new SciHlp(this, SciHlp::_en_role::editor_python);
        splitter->addWidget(shEdit_);
        splitter->setOrientation(Qt::Orientation::Vertical);
    }
    shProt_ = new SciHlp(this, SciHlp::_en_role::prot_macro);
    splitter->addWidget(shProt_);
    container_layout->addWidget(splitter);
    setLayout(container_layout);
    //this->addAction();

    if(_en_role::macro_dlg == en_role_){
        connect(shEdit_, &SciHlp::chngFileInfo, this, &McrWnd::onChngEditFileInfo);

        shEdit_->setContent(R"(
import os
#print(os.get_exec_path())
print(os.getcwd())

@gfg_decorator
def hello_decorator():
    print("Gfg")

class _Xz:
    def __init__(self):
        print('I was born!')
import astra
p = astra.Pet('Rastr')
print(p.getName())
print('that\'s all folks!')
def hllo_wrld():
    print('nice!')
    for i = 0 in range(10):
        print(f'{i} nice more and more')
        #adj = ["red", "big", "tasty"]
        fruits = ["apple", "banana", "cherry"]
        continue
rom datetime import datetime
import pytz

new_york_tz = pytz.timezone('America/New_York')
current_time = datetime.now(new_york_tz)
print(current_time)

pet.bark()

from tkinter import * #sudo apt-get install python3-tk


def about():
    a = Toplevel()
    a.geometry('200x150')
    a['bg'] = 'grey'
    a.overrideredirect(True)
    Label(a, text="About this").pack(expand=1)
    a.after(5000, lambda: a.destroy())

root = Tk()
root.title("Главное окно")
Button(text="Button", width=20).pack()
Label(text="Label", width=20, height=3).pack()
Button(text="About", width=20, command=about).pack()

root.mainloop()

"""
for x in range(6):
  if x == 3: break
  print(x)
else:
  print("Finally finished!")
"""
)");

  shEdit_->setContent(R"(
import os
#print(os.get_exec_path())
print(os.getcwd())

#if from python!
#from astra_py import Rastr, FieldProperties as FP, PropType
#rastr = Rastr()

# для отладки, test.py лежит рядом с  astra.pyd
#sys.path.append(os.getcwd())
sys.path.append(os.path.join(os.path.dirname(__file__),'.'))

rastr.rgm('p1')
#rastr.new_file('')

from tkinter import * #sudo apt-get install python3-tk


def about():
    a = Toplevel()
    a.geometry('200x150')
    a['bg'] = 'grey'
    a.overrideredirect(True)
    Label(a, text="About this").pack(expand=1)
    a.after(5000, lambda: a.destroy())

root = Tk()
root.title("Главное окно")
Button(text="Button", width=20).pack()
Label(text="Label", width=20, height=3).pack()
Button(text="About", width=20, command=about).pack()

root.mainloop()

"""
for x in range(6):
  if x == 3: break
  print(x)
else:
  print("Finally finished!")
"""
)");

    shEdit_->setContent(g_str_tst_rastr_events);
    //shProt_->setContent(R"()");

  }//if(macro_dlg)

    qDebug() << "themeSearchPaths:" << QIcon::themeSearchPaths() << QIcon::themeName();
}

McrWnd::~McrWnd()
{
}

void McrWnd::showEvent(QShowEvent *event)
{
}

void McrWnd::setPyHlp(PyHlp* pPyHlp)
{
  pPyHlp_ = pPyHlp;
}

std::pair<bool,bool> McrWnd::checkSaveModified()
{
    std::pair<bool,bool> pair_saved_cancelled{false,false};
    if(true==shEdit_->getContentModified()){
        QMessageBox msgBox;
        msgBox.setText(tr("Macro modified. Save?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
        msgBox.setDefaultButton(QMessageBox::Yes);
        const QFileInfo fi = shEdit_->getFileInfo();
        switch(msgBox.exec()){
            case QMessageBox::Yes:
                if(fi.absoluteFilePath().length()>3){
                    pair_saved_cancelled.first = onFileSave(false);
                }else{
                    pair_saved_cancelled.first = onFileSave(true);
                }
            break;
            case QMessageBox::No:
                pair_saved_cancelled.first = false;
            break;
            default:
                pair_saved_cancelled.second = true;
        }
    }
    return pair_saved_cancelled;
}

void McrWnd::closeEvent(QCloseEvent *event)
{
    if(pdlgFindRepl_!=nullptr){
        pdlgFindRepl_->close();
        pdlgFindRepl_ = nullptr;
    }
    std::pair<bool,bool> pair_saved_cancelled = checkSaveModified();
    if(pair_saved_cancelled.second==true){
        event->ignore();
        return;
    }
    QMessageBox msgBox;
    msgBox.setText(pair_saved_cancelled.first ? tr("Clear?") : tr("Macro is not saved. Ignore and Exit?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
    msgBox.setDefaultButton(QMessageBox::Yes);
    if(msgBox.exec() != QMessageBox::Yes){
        event->ignore();
        return;
    }
}

void McrWnd::onChngEditFileInfo( const QFileInfo& fiNew)
{
    setWindowTitle(fiNew.absoluteFilePath());
}

bool McrWnd::onFileNew()
{
    qDebug("McrWnd::onFileNew()");
    std::pair<bool,bool> pair_saved_cancelled = checkSaveModified();
    QMessageBox msgBox;
    msgBox.setText(pair_saved_cancelled.first ? tr("Clear?") : tr("Macro is not saved. Ignore and Clear?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
    msgBox.setDefaultButton(QMessageBox::Yes);
    if(msgBox.exec() != QMessageBox::Yes){
        return false;
    }
    shEdit_->setFileInfo(QFileInfo{});
    shEdit_->setContent("");
    return true;
}

void McrWnd::onFileOpen()
{
    qDebug("McrWnd::onFileOpen()");
    bool blContentCleared = true;
    if(true==shEdit_->getContentModified()){
        blContentCleared = onFileNew();
    }
    if(blContentCleared==false){
        QMessageBox msgBox;
        msgBox.setText(tr("Macro is not saved. Ignore and load new?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(msgBox.exec() != QMessageBox::Yes){
            return;
        }
    }
    QString qstrPathToFile = QFileDialog::getOpenFileName(this, tr("Open file"));
    if(qstrPathToFile.length()<3){
        return;
    }
    shEdit_->setFileInfo(QFileInfo{qstrPathToFile});
    const SciHlp::_ret_vals rv = shEdit_->ContentFromFile();
    if(SciHlp::_ret_vals::ok!=rv){
        shEdit_->setFileInfo(QFileInfo{});
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString(tr("Failed load file: %1")).arg( qstrPathToFile )
                       );
        mb.exec();
        return ;
    }
}

bool McrWnd::onFileSave(bool blSaveAs)
{
    qDebug().nospace() << "McrWnd::onFileSave("<< blSaveAs<<")";
    if(blSaveAs == true){
        QString qstrPathToFile = QFileDialog::getSaveFileName(this, tr("Save file as"));
        if(qstrPathToFile.length()<3){
            return false;
        }
        const SciHlp::_ret_vals rv = shEdit_->setFileInfo(QFileInfo(qstrPathToFile));
        if(SciHlp::_ret_vals::ok!=rv){
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString(tr("Failed set file: %1")).arg( qstrPathToFile )
                           );
            mb.exec();
            return false;
        }
    }
    const SciHlp::_ret_vals rv = shEdit_->ContentToFile();
    if(SciHlp::_ret_vals::ok!=rv){
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString("Failed save to file: %1").arg( shEdit_->getFileInfo().absoluteFilePath() )
                       );
        mb.exec();
    }else{
        return true;
    }
    return false;
}

void McrWnd::onRun()
{
  qDebug("McrWnd::onRun()");
  if(nullptr != pPyHlp_){
    const QByteArray qbaTxt{ shEdit_->getText( shEdit_->textLength() ) };
    const PyHlp::enPythonResult PythonResult{ pPyHlp_->Run( qbaTxt.data() ) };
    if( PyHlp::enPythonResult::Ok != PythonResult){
        QString qstrErr;
        if(pPyHlp_->getErrorMessage().length() > 0){
          qstrErr = pPyHlp_->getErrorMessage().c_str();
        } else {
          qstrErr = "UnknownError";
        }
        QMessageBox msgBox;
        msgBox.setText(qstrErr);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(pPyHlp_->getErrorLine() > -1){
          qstrErr += QString("\n Go to line: %1 ?").arg(pPyHlp_->getErrorLine());
          msgBox.setText(qstrErr);
          msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
          if(QMessageBox::Yes == msgBox.exec()){
            shEdit_->gotoLine(pPyHlp_->getErrorLine()-1);
          }
        }else{
          msgBox.exec();
          //msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
        }
    }
  }else{
    shProt_->my_appendText("No PyHlp! \n");
  }
}

void McrWnd::onGoToLine()
{
    const sptr_t n_num_lines = shEdit_->lineCount();
    bool bl_ok = false;
    const int n_go_to_line = QInputDialog::getInt(this, tr("Go to line"), QString(tr("Go to line (1..%1) ")).arg(n_num_lines), 1, 1, n_num_lines + 1, 1, &bl_ok);
    if(bl_ok){
        shEdit_->gotoLine(n_go_to_line - 1);
    }
}

void McrWnd::onFind()
{
    qDebug("McrWnd::onFindRepl()");
    if(pdlgFindRepl_==nullptr){
        pdlgFindRepl_ = new DlgFindRepl(this);
        connect(pdlgFindRepl_, &DlgFindRepl::chngFind, this, &McrWnd::Find);
    }
    pdlgFindRepl_->show();
    pdlgFindRepl_->raise();
    pdlgFindRepl_->activateWindow();
}

void McrWnd::Find(SciHlp::_params_find params_find)
{
    qDebug()<<"Find()-> "<<params_find.qstrFind_  << "\n";
    const SciHlp::_ret_vals rv = shEdit_->Find(params_find);
}

void McrWnd::onProtClear()
{
    shProt_->setContent("");
}

void McrWnd::encode(std::string& data)
{
    std::string buffer;
    buffer.reserve(data.size()+30);
    for(size_t pos = 0; pos != data.size(); ++pos){
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    data.swap(buffer);
}

void McrWnd::encode(std::string& data_out, const QString& qstr_in)
{
    data_out.reserve(qstr_in.length() + 50);
    for(size_t pos = 0; pos != qstr_in.length(); ++pos) {
        if      (QLatin1Char('&')  == qstr_in[static_cast<int>(pos)]){
            data_out.append("&amp;");
        }else if(QLatin1Char('\"') == qstr_in[static_cast<int>(pos)]){
            data_out.append("&quot;");
        }else if(QLatin1Char('\'') == qstr_in[static_cast<int>(pos)]){
            data_out.append("&apos;");
        }else if(QLatin1Char('<')  == qstr_in[static_cast<int>(pos)]){
            data_out.append("&lt;");
        }else if(QLatin1Char('>')  == qstr_in[static_cast<int>(pos)]){
            data_out.append("&gt;");
        }else{
            data_out.append("x", 1);
        }
    }
    data_out.append("\n");
}

void McrWnd::onQStringAppendProtocol(const QString& qstr)
{
    //std::string str{qstr.toStdString()};
    std::string str{""};
    for(int i = 0; i < n_stage_max_id_ ; i++){
        str += "\t";
    }
    str += qstr.toStdString();
    encode(str);
    str += "\n";
    shProt_->my_appendText(str);
}

void McrWnd::onRastrLog(const _log_data& log_data)
{
    std::string str = "";
    for(int i = 0; i < log_data.n_stage_id-1 ; i++){
        str += "\t";
    }
    if( LogMessageTypes::OpenStage == log_data.lmt ){
        str += "<STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\t";
        str += log_data.str_msg;
        str += "\n";
        shProt_->my_appendText(str);
        n_stage_max_id_ = log_data.n_stage_id;
    }
    if( LogMessageTypes::CloseStage == log_data.lmt ){
        str += "</STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\n";
        shProt_->my_appendText(str);
        assert(n_stage_max_id_ == log_data.n_stage_id);
        if(n_stage_max_id_ == log_data.n_stage_id){
            n_stage_max_id_--;
        }
    }
}

void McrWnd::onRastrPrint(const std::string& str_msg)
{
    shProt_->my_appendText(str_msg);
}
