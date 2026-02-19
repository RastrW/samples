#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
}
MainWindow::~MainWindow(){
}
#include "mcrwnd.h"
void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
    if(pMcrWnd_ == nullptr){
        pMcrWnd_ = new McrWnd(this);
        pMcrWnd_->show();
    }
};
