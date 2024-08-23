#include <QSplitter>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <QCloseEvent>
#include "mcrwnd.h"
#include "scihlp.h"
#include "tst_toolbox.h"
#include "tst2_dialog.h"
#include "forms/dlgfindrepl.h"

#include <QQmlDebuggingEnabler>
QQmlDebuggingEnabler enabler;

McrWnd::McrWnd(QWidget* parent)
    : QDialog(parent,
              Qt::WindowMinimizeButtonHint |
              Qt::WindowMaximizeButtonHint |
              Qt::WindowCloseButtonHint){
    const int nWidth = 600;
    const int nHeight = 800;
    resize(nWidth, nHeight);
    setWindowIcon( QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon) ));
    setWindowTitle(tr("Macro Python"));
    QSplitter* splitter = new QSplitter(this);
    shEdit_ = new SciHlp(this, SciHlp::_en_role::editor_python);
    shProt_ = new SciHlp(this, SciHlp::_en_role::prot_macro);
    QVBoxLayout* layout = new QVBoxLayout();
    QVBoxLayout* container_layout = new QVBoxLayout();
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
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)),    tr("&FindRepl"), this, SLOT( onFindRepl() )   )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_F)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_CommandLink)),      tr("&Go to line"), this, SLOT( onGoToLine() )  )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_G)});
    container_layout->addWidget(pToolBar);
    splitter->addWidget(shEdit_);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(shProt_);
    container_layout->addWidget(splitter);
    setLayout(container_layout);
    //this->addAction();

    connect( shEdit_, SIGNAL( chngFileInfo(const QFileInfo&) ), this, SLOT( onChngEditFileInfo(const QFileInfo&) ) );
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

    qDebug() << "themeSearchPaths:" << QIcon::themeSearchPaths() << QIcon::themeName();

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
    /*if(true==shEdit_->getContentModified()){
        QMessageBox msgBox;
        msgBox.setText(tr("Macro modified. Save?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(msgBox.exec() == QMessageBox::Yes){
            QFileInfo fi = shEdit_->getFileInfo();
            if(fi.absoluteFilePath().length()>3){
                blFileSaved = onFileSave(false);
            }else{
                blFileSaved = onFileSave(true);
            }
        }
    }*/
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
}
void McrWnd::onGoToLine(){
    const sptr_t n_num_lines = shEdit_->lineCount();
    bool bl_ok = false;
    const int n_go_to_line = QInputDialog::getInt(this, tr("Line number"), QString(tr("Go to line (1..%1) ")).arg(n_num_lines), 1, 1, n_num_lines + 1, 1, &bl_ok);
    if(bl_ok){
        shEdit_->gotoLine(n_go_to_line - 1);
    }
}
void McrWnd::onFindRepl(){
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
