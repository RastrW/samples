#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>
#include <QDialog>

//class McrWnd : public QWidget{
class McrWnd : public QDialog{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent = nullptr);
    virtual ~McrWnd();
    void showEvent(QShowEvent *event) override;


signals:

};

#endif // MCRWND_H
