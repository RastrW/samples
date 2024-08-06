#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>

class McrWnd : public QWidget
{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent = nullptr);
    virtual ~McrWnd();

signals:

};

#endif // MCRWND_H
