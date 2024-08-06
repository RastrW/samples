#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class McrWnd;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();
    void showEvent(QShowEvent *event) override;
    McrWnd* pMcrWnd_;
};
#endif // MAINWINDOW_H
