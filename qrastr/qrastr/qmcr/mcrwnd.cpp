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
#include "../qastra_events_data.h"

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
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)),        tr("Run (F5)"),  this, SLOT( onRun() )        )
                ->setShortcut({QKeySequence(Qt::Key_F5)});
        //pToolBar->addWidget(leFind);
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)),    tr("&Find"),     this, SLOT( onFind() )       )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_F)});
        pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink)),      tr("&Go to line"), this, SLOT( onGoToLine() ) )
                ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_G)});
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
        connect( shEdit_, SIGNAL( chngFileInfo( const QFileInfo& ) ), this, SLOT( onChngEditFileInfo( const QFileInfo& ) ) );

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

    shProt_->setContent(R"(
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

)");
    }//if(macro_dlg)

    qDebug() << "themeSearchPaths:" << QIcon::themeSearchPaths() << QIcon::themeName();
/*
    QIcon::setThemeName("oxygen");
    tst_tb_ = new Tst_ToolBox(this->parentWidget());
    //tst_tb_->setHidden(true);
    tst_tb_->show();
    //tst_tb_->move(1500,600);
    //tst_tb_->stackUnder(this->parentWidg

    //this->show();
    //this->raise();
    //this->activateWindow();

    tst2_dlg_ = new Tst2_Dialog(this);
    //tst2_dlg_->show();
*/
}
McrWnd::~McrWnd(){
}
void McrWnd::showEvent(QShowEvent *event) {
}
std::pair<bool,bool> McrWnd::checkSaveModified(){
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
void McrWnd::closeEvent(QCloseEvent *event) {
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
void McrWnd::onChngEditFileInfo( const QFileInfo& fiNew){
    setWindowTitle(fiNew.absoluteFilePath());
}
bool McrWnd::onFileNew(){
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
void McrWnd::onFileOpen(){
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
bool McrWnd::onFileSave(bool blSaveAs){
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
void McrWnd::onRun(){
    qDebug("McrWnd::onRun()");
    for( int i = 1 ; i < 100000 ; i++ ){
        shProt_->my_appendTect( "<ываываыва> ЫВАЫВА фывfdsfsdafsd ываыв [" + std::to_string(i)+  "] ыввввЁЁЁ  ываывпике \n  </ываываыва> \n");
    }
}
void McrWnd::onGoToLine(){
    const sptr_t n_num_lines = shEdit_->lineCount();
    bool bl_ok = false;
    const int n_go_to_line = QInputDialog::getInt(this, tr("Go to line"), QString(tr("Go to line (1..%1) ")).arg(n_num_lines), 1, 1, n_num_lines + 1, 1, &bl_ok);
    if(bl_ok){
        shEdit_->gotoLine(n_go_to_line - 1);
    }
}
void McrWnd::onFind(){
    qDebug("McrWnd::onFindRepl()");
    if(pdlgFindRepl_==nullptr){
        pdlgFindRepl_ = new DlgFindRepl(this);
        connect( pdlgFindRepl_, SIGNAL( chngFind( SciHlp::_params_find ) ), this, SLOT( Find( SciHlp::_params_find ) ) );
    }
    pdlgFindRepl_->show();
    pdlgFindRepl_->raise();
    pdlgFindRepl_->activateWindow();
}
void McrWnd::Find(SciHlp::_params_find params_find){
    qDebug()<<"Find()-> "<<params_find.qstrFind_  << "\n";
    const SciHlp::_ret_vals rv = shEdit_->Find(params_find);
}
void encode(std::string& data){
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
void encode(std::string& data_out, const QString& qstr_in){
    data_out.reserve(qstr_in.length() + 50);
    for(size_t pos = 0; pos != qstr_in.length(); ++pos) {
        if      (QLatin1Char('&')  == qstr_in[pos]){
            data_out.append("&amp;");
        }else if(QLatin1Char('\"') == qstr_in[pos]){
            data_out.append("&quot;");
        }else if(QLatin1Char('\'') == qstr_in[pos]){
            data_out.append("&apos;");
        }else if(QLatin1Char('<')  == qstr_in[pos]){
            data_out.append("&lt;");
        }else if(QLatin1Char('>')  == qstr_in[pos]){
            data_out.append("&gt;");
        }else{
            data_out.append("x", 1);
        }
    }
    data_out.append("\n");
}
void McrWnd::onQStringAppendProtocol(const QString& qstr){
    std::string str{qstr.toStdString()};
    encode(str);
    str += "\n";
    shProt_->my_appendTect(str);
}
void McrWnd::onRastrLog(const _log_data& log_data){
    std::string str = "";
    if( LogMessageTypes::OpenStage == log_data.lmt) {
        str  = "<STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\t";
        str += log_data.str_msg;
        str += "\n";
        shProt_->my_appendTect(str);
    }
    if( LogMessageTypes::CloseStage == log_data.lmt) {
        str  = "</STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">";
        shProt_->my_appendTect(str);
    }
}
