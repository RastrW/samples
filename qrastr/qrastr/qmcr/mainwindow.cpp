#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
}
MainWindow::~MainWindow(){
}
#include "mcrwnd.h"
void MainWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent( event );
    pMcrWnd_ = new McrWnd(this);
    pMcrWnd_->show();
/*
    QWidget* w = new QWidget();
    w->show();*/
};
