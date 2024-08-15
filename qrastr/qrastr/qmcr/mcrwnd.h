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
//    QToolBar* pToolBar_ = nullptr;
   // QAction* pActNew_ = nullptr;
    /*
    QAction* pActSave_ = nullptr;
    QAction* pActPlay_ = nullptr;
    QAction* pActFindRepl_ = nullptr;
    */
signals:
private slots:
    void onNewFile();
    void onOpenFile();
    void onSave();
    void onRun();
    void onFindRepl();
private:

};

#endif // MCRWND_H
