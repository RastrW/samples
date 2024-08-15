#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>
#include <QDialog>
#include <QToolBar>

class McrWnd : public QDialog{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent = nullptr);
    virtual ~McrWnd();
    void showEvent(QShowEvent *event) override;
signals:
private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave(bool blSaveAs);
    void onRun();
    void onFindRepl();
private:
};

#endif // MCRWND_H
