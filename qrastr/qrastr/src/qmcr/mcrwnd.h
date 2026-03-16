#pragma once

#include <QWidget>
#include <QDialog>
#include <QToolBar>
#include <QFileInfo>
#include "sciPyEditor.h"
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
public slots:
    void slot_rastrLog  (const _log_data&   logData);
    void slot_rastrPrint(const std::string& message);
private slots:
    void slot_fileNew ();
    bool slot_fileSave (bool saveAs);
    void slot_fileOpen ();
    void slot_run      ();
    void slot_goToLine ();
    void slot_find     ();
    void slot_protClear();

    void slot_fileInfoChanged(const QFileInfo& fi);
    void slot_findByParams   (SciPyEditor::FindParams params);
    void slot_appendProtocol (const QString& text);
private:
    // Результат диалога «сохранить изменения?»
    enum class SavePromptResult { NoChanges, Saved, Discarded, Cancelled };
    SavePromptResult promptSaveIfModified();

    void buildToolBar();

    SciPyEditor*             m_editor{nullptr};
    ProtocolLogWidget*  m_logWidget{ nullptr };
    DlgFindRepl*        m_dlgFind{ nullptr};
    QVBoxLayout*        m_mainLayout  {nullptr};

    std::shared_ptr<PyHlp> m_pyHlp;
    bool m_firstShow {true};
};
