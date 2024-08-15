#include <QSplitter>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
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
    SciHlp* edit1 = new SciHlp(this, SciHlp::_en_role::editor_python);
    edit1->setContent(R"(
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
"""
for x in range(6):
  if x == 3: break
  print(x)
else:
  print("Finally finished!")
"""
                   )");
    SciHlp* edit2 = new SciHlp(this, SciHlp::_en_role::prot_macro);
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *container_layout = new QVBoxLayout;
    QToolBar* pToolBar = nullptr;
    pToolBar  = new QToolBar;
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)),               tr("&New"),      QKeySequence(Qt::CTRL+Qt::Key_N), this, SLOT( onFileNew())   );
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogStart)),        tr("&Open"),     QKeySequence(Qt::CTRL+Qt::Key_O), this, SLOT( onFileOpen())  );
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton)),       tr("&Save"),     QKeySequence(Qt::CTRL+Qt::Key_S), this, [this]{ onFileSave(false); } );
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)),            tr("Save as"),   QKeySequence(Qt::CTRL+Qt::Key_W), this, [this]{ onFileSave(true);  } );
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay)),              tr("&Run"),      QKeySequence(Qt::Key_F5),         this, SLOT( onRun() )      );
    pToolBar->addAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView)), tr("&FindRepl"), QKeySequence(Qt::CTRL+Qt::Key_F), this, SLOT( onFindRepl() ) );
    container_layout->addWidget(pToolBar);
    splitter->addWidget(edit1);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(edit2);
    container_layout->addWidget(splitter);
    setLayout(container_layout);
}
McrWnd::~McrWnd(){
}
void McrWnd::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
}
void McrWnd::onFileNew(){
    qDebug("McrWnd::onFileNew()");
}
void McrWnd::onFileOpen(){
    qDebug("McrWnd::onFileOpen()");
}
void McrWnd::onFileSave(bool blSaveAs){
    qDebug().nospace() << "McrWnd::onFileSave("<< blSaveAs<<")";
}
void McrWnd::onRun(){
    qDebug("McrWnd::onRun()");
}
void McrWnd::onFindRepl(){
    qDebug("McrWnd::onFindRepl()");
}
