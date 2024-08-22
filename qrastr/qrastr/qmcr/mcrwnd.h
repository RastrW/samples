#ifndef MCRWND_H
#define MCRWND_H

#include <QWidget>
#include <QDialog>
#include <QToolBar>
#include <QFileInfo>
#include "scihlp.h"

class  Tst_ToolBox;
class  Tst2_Dialog;
class  DlgFindRepl;

class McrWnd : public QDialog{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent = nullptr);
    virtual ~McrWnd();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
signals:
private slots:
    std::pair<bool,bool> checkSaveModified();
    bool onFileNew();
    void onFileOpen();
    bool onFileSave(bool blSaveAs);
    void onRun();
    void onFindRepl();
    void onChngEditFileInfo(const QFileInfo& fiNew);
    void FindRepl(SciHlp::_params_findrepl params_findrepl);
private:
    SciHlp*       shEdit_{nullptr};
    SciHlp*       shProt_{nullptr};
    Tst_ToolBox*  tst_tb_{nullptr};
    Tst2_Dialog*  tst2_dlg_{ nullptr};
    DlgFindRepl*  pdlgFindRepl_{ nullptr};
};

#endif // MCRWND_H
