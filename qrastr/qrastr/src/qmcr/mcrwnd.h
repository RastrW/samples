#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QToolBar>
#include <QFileInfo>
#include <QPointer>
#include "sciPyEditor.h"
#include "qmcr_api.h"


class  DlgFindRepl;
class  PyHlp;
struct _log_data;
class GlobalProtocolWidget;
class QVBoxLayout;
class GlobalProtocolWidget;
class QLabel;

class QMCR_API McrWnd : public QWidget {
    Q_OBJECT
public:
    explicit McrWnd(QWidget *parent);
    virtual ~McrWnd();

    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);

    /**
     * @brief Запросить сохранение если есть изменения.
     * @return true  — можно закрывать (нет изменений / сохранено / отброшено)
     *         false — пользователь нажал «Отмена»
     */
    bool promptAndAllowClose();
signals:
    /// Испускается при смене файла; MacroDockManager обновит заголовок вкладки.
    void titleChanged(const QString& title);
public slots:
    ///@todo нужен ли этот слот, он не используется?
    void slot_rastrLog  (const _log_data&   logData);
    void slot_rastrPrint(const std::string& message);
private slots:
    void slot_fileNew ();
    bool slot_fileSave ();
    bool slot_fileSaveAs();
    void slot_fileOpen ();
    void slot_run      ();
    void slot_goToLine ();
    void slot_find     ();
    void slot_protClear();

    void slot_fileInfoChanged(const QFileInfo& fi);
    void slot_findByParams   (SciPyEditor::FindParams params);
    ///@todo нужен ли этот слот, он не используется?
    void slot_appendProtocol (const QString& text);

    void slot_updateStatusBar();
private:
    // Результат диалога «сохранить изменения?»
    enum class SavePromptResult { NoChanges, Saved, Discarded, Cancelled };
    SavePromptResult promptSaveIfModified();

    SciPyEditor*        m_editor{nullptr};
    GlobalProtocolWidget*      m_glodLogWidget{ nullptr };

    QMenuBar*   m_menuBar   {nullptr};
    QToolBar*   m_toolBar   {nullptr};
    QStatusBar* m_statusBar {nullptr};

    QPointer<DlgFindRepl>
        m_findDlg;    //хранится между вызовами

    //виджеты статусбара
    QLabel* m_sbFile    {nullptr};
    QLabel* m_sbPos     {nullptr};
    QLabel* m_sbModified{nullptr};

    std::shared_ptr<PyHlp> m_pyHlp;
    bool m_firstShow {true};

    void buildMenuAndToolBar();
    void buildStatusBar();
};
