#include <QSplitter>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "mcrwnd.h"
#include "scihlp.h"

McrWnd::McrWnd(QWidget *parent)
    : QDialog(parent,
              Qt::WindowMinimizeButtonHint |
              Qt::WindowMaximizeButtonHint |
              Qt::WindowCloseButtonHint){
    const int nWidth = 600;
    const int nHeight = 800;
    resize(nWidth, nHeight);
    setWindowIcon( QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon) ));
    setWindowTitle(tr("Macro Python"));
    QSplitter * splitter = new QSplitter(this);
    shEdit_ = new SciHlp(this, SciHlp::_en_role::editor_python);
    shProt_ = new SciHlp(this, SciHlp::_en_role::prot_macro);
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *container_layout = new QVBoxLayout;
    QToolBar* pToolBar = nullptr;
    pToolBar  = new QToolBar;
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)),               tr("&New"),     this,    SLOT( onFileNew())   )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_N)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon)),                tr("&Open"),    this,   SLOT( onFileOpen())   )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_O)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton)),       tr("&Save"),    [this] { onFileSave(false); } )
            ->setShortcuts( {QKeySequence(Qt::CTRL+Qt::Key_S)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)),            tr("Save as"),  [this] { onFileSave(true); }  )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_W)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)),              tr("Run (F5)"),  this, SLOT( onRun() )        )
            ->setShortcut({QKeySequence(Qt::Key_F5)});
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)),          tr("&FindRepl"), this, SLOT( onFindRepl() )   )
            ->setShortcut({QKeySequence(Qt::CTRL+Qt::Key_F)});
    container_layout->addWidget(pToolBar);
    splitter->addWidget(shEdit_);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(shProt_);
    container_layout->addWidget(splitter);
    setLayout(container_layout);

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

}
McrWnd::~McrWnd(){
}
void McrWnd::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
}
void McrWnd::onChngEditFileInfo( const QFileInfo& fiNew){
    setWindowTitle(fiNew.absoluteFilePath());
}
void McrWnd::onFileNew(){
    qDebug("McrWnd::onFileNew()");
}
void McrWnd::onFileOpen(){
    qDebug("McrWnd::onFileOpen()");
}
void McrWnd::onFileSave(bool blSaveAs){
    qDebug().nospace() << "McrWnd::onFileSave("<< blSaveAs<<")";
    if(blSaveAs == true){
        QString qstrPathToFile = QFileDialog::getSaveFileName(this, tr("Save File"));
        const SciHlp::_ret_vals rv = shEdit_->setFileInfo(QFileInfo(qstrPathToFile));
        if(SciHlp::_ret_vals::ok!=rv){
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString("Failed open file: %1").arg( qstrPathToFile )
                           );
            mb.exec();
            return;
        }
    }
    const SciHlp::_ret_vals rv = shEdit_->ContentToFile();
    if(SciHlp::_ret_vals::ok!=rv){
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString("Failed save to file: %1").arg( shEdit_->getFileInfo().absoluteFilePath() )
                       );
        mb.exec();
    }
}
void McrWnd::onRun(){
    qDebug("McrWnd::onRun()");
}
void McrWnd::onFindRepl(){
    qDebug("McrWnd::onFindRepl()");
}
