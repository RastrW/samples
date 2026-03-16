#include <QSplitter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <QCloseEvent>

#include "mcrwnd.h"
#include "forms/dlgfindrepl.h"
#include "../app/astra/qastra_events_data.h"
#include "pyhlp.h"
#include "protocolLogWidget.h"

McrWnd::McrWnd(QWidget* parent)
    : QDialog(parent,
              Qt::WindowMinimizeButtonHint |
              Qt::WindowMaximizeButtonHint |
              Qt::WindowCloseButtonHint){
    resize(600, 800);
    setWindowIcon( QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon) ));
    setWindowTitle(tr("Macro Python"));

    m_mainLayout = new QVBoxLayout(this);

    buildToolBar();

    auto* splitter = new QSplitter(Qt::Vertical, this);
    m_editor = new SciPyEditor(this);
    m_logWidget = new ProtocolLogWidget(this);

    splitter->addWidget(m_editor);
    splitter->addWidget(m_logWidget);

    m_mainLayout->addWidget(splitter);
    setLayout(m_mainLayout);

    connect(m_editor, &SciPyEditor::sig_fileInfoChanged,
            this, &McrWnd::slot_fileInfoChanged);
}

McrWnd::~McrWnd() = default;

void McrWnd::buildToolBar()
{
    auto* tb = new QToolBar(this);
    tb->setIconSize(QSize(16, 16));

    auto addBtn = [&](QStyle::StandardPixmap icon, const QString& text,
                      auto slot, QKeySequence shortcut = {})
    {
        auto* act = tb->addAction(
            QIcon(QApplication::style()->standardIcon(icon)), text,
            this, slot);
        if (!shortcut.isEmpty())
            act->setShortcut(shortcut);
    };

    addBtn(QStyle::SP_FileIcon,          tr("&New"),      &McrWnd::slot_fileNew,
           QKeySequence(Qt::CTRL | Qt::Key_N));
    addBtn(QStyle::SP_DirIcon,           tr("&Open"),     &McrWnd::slot_fileOpen,
           QKeySequence(Qt::CTRL | Qt::Key_O));
    addBtn(QStyle::SP_DialogSaveButton,  tr("&Save"),
           [this]{ slot_fileSave(false); },
           QKeySequence(Qt::CTRL | Qt::Key_S));
    addBtn(QStyle::SP_DriveFDIcon,       tr("Save &as"),
           [this]{ slot_fileSave(true);  },
           QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));

    tb->addSeparator();
    addBtn(QStyle::SP_MediaPlay,         tr("Run (F10)"), &McrWnd::slot_run,
           Qt::Key_F10);

    tb->addSeparator();
    addBtn(QStyle::SP_BrowserReload,     tr("&Find"),     &McrWnd::slot_find,
           QKeySequence(Qt::CTRL | Qt::Key_F));
    addBtn(QStyle::SP_CommandLink,       tr("&Go to line"), &McrWnd::slot_goToLine,
           QKeySequence(Qt::CTRL | Qt::Key_G));

    tb->addSeparator();
    addBtn(QStyle::SP_DialogResetButton, tr("&Clear log"), &McrWnd::slot_protClear);

    m_mainLayout->addWidget(tb);
}

void McrWnd::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    if (!m_firstShow)
        return;
    m_firstShow = false;

    const auto btn = QMessageBox::question(
        this, tr("Macro Python"),
        tr("Load example script?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (btn != QMessageBox::Yes)
        return;

    const QString examplePath =
        QCoreApplication::applicationDirPath() + "/../Data/py_example/rastr_events.py";

    m_editor->setFileInfo(QFileInfo(examplePath));
    if (SciHlpBase::RetVal::Ok != m_editor->loadFromFile()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Example file not found:\n%1").arg(examplePath));
        m_editor->setFileInfo(QFileInfo{});
    }
}

void McrWnd::setPyHlp(std::shared_ptr<PyHlp> pPyHlp)
{
  m_pyHlp = pPyHlp;
}

McrWnd::SavePromptResult McrWnd::promptSaveIfModified()
{
    if (!m_editor->isModified())
        return SavePromptResult::NoChanges;

    QMessageBox mb(QMessageBox::Question,
                   tr("Macro Python"),
                   tr("The macro has unsaved changes. Save?"),
                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                   this);
    mb.setDefaultButton(QMessageBox::Yes);

    switch (mb.exec()) {
    case QMessageBox::Yes: {
        const bool hasPath =
            m_editor->getFileInfo().absoluteFilePath().length() > 3;
        return slot_fileSave(!hasPath)
                   ? SavePromptResult::Saved
                   : SavePromptResult::Cancelled;
    }
    case QMessageBox::No:
        return SavePromptResult::Discarded;
    default:
        return SavePromptResult::Cancelled;
    }
}

void McrWnd::closeEvent(QCloseEvent* event)
{
    if (m_dlgFind) {
        m_dlgFind->close();
    }

    const auto result = promptSaveIfModified();
    if (result == SavePromptResult::Cancelled) {
        event->ignore();
        return;
    }

    QDialog::closeEvent(event);
}

void McrWnd::slot_fileNew()
{
    const auto result = promptSaveIfModified();
    if (result == SavePromptResult::Cancelled)
        return;

    m_editor->setFileInfo(QFileInfo{});
    m_editor->setContent("");
}

void McrWnd::slot_fileOpen()
{
    const auto result = promptSaveIfModified();
    if (result == SavePromptResult::Cancelled)
        return;

    const QString path = QFileDialog::getOpenFileName(this, tr("Open file"));
    if (path.length() < 3)
        return;

    m_editor->setFileInfo(QFileInfo{path});
    if (SciHlpBase::RetVal::Ok != m_editor->loadFromFile()) {
        m_editor->setFileInfo(QFileInfo{});
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to load file:\n%1").arg(path));
    }
}

bool McrWnd::slot_fileSave(bool saveAs)
{
    if (saveAs) {
        const QString path = QFileDialog::getSaveFileName(this, tr("Save file as"));
        if (path.length() < 3)
            return false;
        if (SciHlpBase::RetVal::Ok != m_editor->setFileInfo(QFileInfo(path))) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Cannot set file path:\n%1").arg(path));
            return false;
        }
    }

    if (SciHlpBase::RetVal::Ok != m_editor->saveToFile()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to save file:\n%1")
                                  .arg(m_editor->getFileInfo().absoluteFilePath()));
        return false;
    }
    return true;
}

void McrWnd::slot_run()
{
    if (!m_pyHlp) {
        m_logWidget->onRastrPrint("No PyHlp!\n");
        return;
    }

    const QByteArray src = m_editor->getText(m_editor->textLength());
    const PyHlp::Result res = m_pyHlp->run(src.constData());

    if (res == PyHlp::Result::Ok)
        return;

    const QString errMsg = m_pyHlp->getErrorMessage().empty()
                               ? tr("Unknown error")
                               : QString::fromStdString(m_pyHlp->getErrorMessage());

    const long errLine = m_pyHlp->getErrorLine();

    QMessageBox mb(QMessageBox::Critical, tr("Python error"), errMsg, {}, this);
    if (errLine > -1) {
        mb.setInformativeText(tr("Go to line %1?").arg(errLine));
        mb.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        if (mb.exec() == QMessageBox::Yes)
            m_editor->gotoLine(errLine - 1);
    } else {
        mb.setStandardButtons(QMessageBox::Ok);
        mb.exec();
    }
}

void McrWnd::slot_goToLine()
{
    const sptr_t total = m_editor->lineCount();
    bool ok = false;
    const int line = QInputDialog::getInt(
        this, tr("Go to line"),
        tr("Line number (1..%1):").arg(total),
        1, 1, static_cast<int>(total), 1, &ok);
    if (ok)
        m_editor->gotoLine(line - 1);
}

void McrWnd::slot_find()
{
    if (!m_dlgFind) {
        m_dlgFind = new DlgFindRepl(this);
        m_dlgFind->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_dlgFind, &DlgFindRepl::sig_find,
                this, &McrWnd::slot_findByParams);
    }

    m_dlgFind->show();
    m_dlgFind->raise();
    m_dlgFind->activateWindow();
}

void McrWnd::slot_findByParams(SciPyEditor::FindParams params_find)
{
    if (SciPyEditor::RetVal::Ok != m_editor->find(params_find)) {
        QMessageBox::information(this, tr("Find"),
                                 tr("'%1' not found.").arg(params_find.m_text));
    }
}

void McrWnd::slot_protClear()
{
    m_logWidget->clear();
}

void McrWnd::slot_appendProtocol(const QString& qstr)
{
    m_logWidget->onAppendText(qstr);
}

void McrWnd::slot_rastrLog(const _log_data& log_data){
    m_logWidget->onRastrLog(log_data);
}

void McrWnd::slot_rastrPrint(const std::string& str_msg){
    m_logWidget->onRastrPrint(str_msg);
}

void McrWnd::slot_fileInfoChanged( const QFileInfo& fiNew)
{
    setWindowTitle(fiNew.absoluteFilePath().isEmpty()
                   ? tr("Macro Python")
                   : fiNew.absoluteFilePath());
}
