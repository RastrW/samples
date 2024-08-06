#include <QSplitter>
#include <QVBoxLayout>
#include <QTextEdit>
#include "mcrwnd.h"

McrWnd::McrWnd(QWidget *parent)
    //: QWidget{parent}{
    : QDialog{parent}{
}
McrWnd::~McrWnd(){
}
void McrWnd::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
    setWindowTitle(tr("Macro Python"));
    QSplitter * splitter = new QSplitter(this);
    QTextEdit* edit1 = new QTextEdit();
    QTextEdit* edit2 = new QTextEdit();
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *container_layout = new QVBoxLayout;
    splitter->addWidget(edit1);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->addWidget(edit2);
    container_layout->addWidget(splitter);
    setLayout(container_layout);
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
