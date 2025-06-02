#ifndef MCRWND_H
#define MCRWND_H
#pragma once

#include <QWidget>
#include <QDialog>
#include <QToolBar>
#include <QFileInfo>
#include "scihlp.h"

class  Tst_ToolBox;
class  Tst2_Dialog;
class  DlgFindRepl;
class  PyHlp;
struct _log_data;
class McrWnd : public QDialog{
    Q_OBJECT
public:
    enum class _en_role{
        macro_dlg       = 1,
        global_protocol = 2
    };
    explicit McrWnd(QWidget *parent, const _en_role en_role);
    virtual ~McrWnd();
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void encode(std::string& data);
    void encode(std::string& data_out, const QString& qstr_in);
    void setPyHlp(PyHlp* pPyHlp);
signals:
private slots:
    std::pair<bool,bool> checkSaveModified();
    bool onFileNew();
    void onFileOpen();
    bool onFileSave(bool blSaveAs);
    void onRun();
    void onGoToLine();
    void onFind();
    void onProtClear();
    void onChngEditFileInfo(const QFileInfo& fiNew);
    void Find(SciHlp::_params_find params_find);
    void onQStringAppendProtocol(const QString& qstr);
    void onRastrLog(const _log_data&);
    void onRastrPrint(const std::string&);
private:
    const _en_role en_role_;
    SciHlp*        shEdit_{nullptr};
    SciHlp*        shProt_{nullptr};
    Tst_ToolBox*   tst_tb_{nullptr};
    Tst2_Dialog*   tst2_dlg_{ nullptr};
    DlgFindRepl*   pdlgFindRepl_{ nullptr};
    long           n_stage_max_id_ = 0;
    PyHlp*         pPyHlp_{nullptr};
};

#endif // MCRWND_H
