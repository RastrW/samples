#pragma once

#include <QWidget>
#include <QDialog>
#include <QToolBar>
#include <QFileInfo>
#include "scihlp.h"
#include "qmcr_api.h"

class  DlgFindRepl;
class  PyHlp;
struct _log_data;
class ProtocolLogWidget;
class QVBoxLayout;

class QMCR_API McrWnd : public QDialog{
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent);
    virtual ~McrWnd();

    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);
signals:
public slots:
    void onRastrLog(const _log_data&);
    void onRastrPrint(const std::string&);
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
private:
    void buildToolBar();

    SciHlp*        shEdit_{nullptr};
    ProtocolLogWidget* m_logWidget{ nullptr };
    DlgFindRepl*   pdlgFindRepl_{ nullptr};

    QVBoxLayout* m_containerLayout;
    std::shared_ptr<PyHlp> pPyHlp_;
};
