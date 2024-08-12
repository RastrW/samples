#include <QSplitter>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include "mcrwnd.h"
//#include "ScintillaEdit.h"
#include "scihlp.h"


McrWnd::McrWnd(QWidget *parent)
    //: QWidget{parent}{
    : QDialog(parent,
              Qt::WindowMinimizeButtonHint |
              Qt::WindowMaximizeButtonHint |
              Qt::WindowCloseButtonHint){
    const int nWidth = 600;
    const int nHeight = 800;
    resize(nWidth, nHeight);

  //  pActNew_ = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
     //action->setIcon( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)) );
    //  pActNew_ = new QAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon)),tr("&New"), this);
    pActNew_ = new QAction( QIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogStart )),tr("&New"), this);
    pActNew_->setShortcut(tr("Ctrl+N")); //SP_DialogSaveButton, SP_MediaPlay,SP_FileDialogContentsView
    //action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    pActNew_->setStatusTip(tr("Create a new file"));
    connect( pActNew_, SIGNAL(triggered()), this, SLOT(newFile()));

}
McrWnd::~McrWnd(){
}
void McrWnd::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
    setWindowTitle(tr("Macro Python"));
    QSplitter * splitter = new QSplitter(this);
  // QTextEdit* edit1 = new QTextEdit();
  //  ScintillaEdit* edit1 = new ScintillaEdit;
    SciHlp* edit1 = new SciHlp(nullptr, SciHlp::_en_role::editor_python);
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
    //QTextEdit* edit2 = new QTextEdit();
    //ScintillaEdit* edit2 = new ScintillaEdit;
    SciHlp* edit2 = new SciHlp(nullptr, SciHlp::_en_role::prot_macro);
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *container_layout = new QVBoxLayout;
//    toolbar = new QToolBar;
  //  toolLayout->addWidget(toolbar);
    pToolBar_  = new QToolBar;
    pToolBar_->addAction(pActNew_);
    container_layout->addWidget(pToolBar_);

    splitter->addWidget(edit1);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(edit2);
    container_layout->addWidget(splitter);
    setLayout(container_layout);
/*
    toolLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    //set margins to zero so the toolbar touches the widget's edges
    toolLayout->setContentsMargins(0, 0, 0, 0);

    toolbar = new QToolBar;
    toolLayout->addWidget(toolbar);

    //use a different layout for the contents so it has normal margins
    contentsLayout = new ...
    toolLayout->addLayout(contentsLayout);

*/

/*
    QWidget* pWndWithSplitter = new QWidget();
    QSplitter * splitter = new QSplitter(pWndWithSplitter);
    Buffer* edit1 = new Buffer();
    Buffer* edit2 = new Buffer();
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *container_layout = new QVBoxLayout;
    splitter->addWidget(edit1);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(edit2);
    container_layout->addWidget(splitter);
    pWndWithSplitter->setLayout(container_layout);
    pWndWithSplitter->show();
    */

}
void McrWnd::newFile(){
    printf("sddfdf");
    qDebug("new File");
}
