#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>
#include <QDialog>
#include <QToolBar>
#include <QFileInfo>

class SciHlp;

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
    void onChngEditFileInfo(const QFileInfo& fiNew);
private:
    SciHlp* shEdit_ = nullptr;
    SciHlp* shProt_ = nullptr;
};

#endif // MCRWND_H
