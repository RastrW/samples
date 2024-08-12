#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>
#include <QDialog>
#include <QToolBar>

//class McrWnd : public QWidget{
class McrWnd : public QDialog{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent = nullptr);
    virtual ~McrWnd();
    void showEvent(QShowEvent *event) override;
    QToolBar* pToolBar_ = nullptr;
    QAction* pActNew_ = nullptr;


signals:
private slots:
    void newFile();

};

#endif // MCRWND_H
