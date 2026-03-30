#include <QSplitter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QCloseEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include "forms/dlgfindrepl.h"

#include "mcrwnd.h"

#include "../app/astra/qastra_events_data.h"
#include "pyhlp.h"
#include "globalProtocolWidget.h"
#include <spdlog/spdlog.h>

McrWnd::McrWnd(QWidget* parent)
    : QMainWindow(parent)
{
    resize(600, 800);
    setWindowIcon(QIcon(":/images/new_style/python.png"));
    setWindowTitle(tr("Macro Editor — Python"));

    auto* central  = new QWidget(this);
    auto* layout   = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* splitter = new QSplitter(Qt::Vertical, central);
    m_editor        = new SciPyEditor(central);
    m_glodLogWidget = new GlobalProtocolWidget(central);

    splitter->addWidget(m_editor);
    splitter->addWidget(m_glodLogWidget);
    layout->addWidget(splitter);
    setCentralWidget(central);

    buildMenuAndToolBar();
    buildStatusBar();

    connect(m_editor, &SciPyEditor::sig_fileInfoChanged,
            this, &McrWnd::slot_fileInfoChanged);

    //Scintilla сигнализирует об изменении позиции через updateUi
    connect(m_editor, &ScintillaEdit::updateUi,
            this, [this](Scintilla::Update){ slot_updateStatusBar(); });

    spdlog::info("McrWnd loaded");
}

McrWnd::~McrWnd() = default;

void McrWnd::buildMenuAndToolBar()
{
    // Фабрика: создаёт QAction, присваивает иконку, текст, шорткат, слот
    auto mkAct = [&](const QString& icon, const QString& text,
                     auto slot, QKeySequence sc = {}) -> QAction*
    {
        auto* a = new QAction(QIcon(icon), text, this);
        if (!sc.isEmpty())
            a->setShortcut(sc);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    // ── Actions ──────────────────────────────────────────────────────────────
    auto* actNew    = mkAct(":/images/new_style/document.png",       tr("&Новый"),
                         &McrWnd::slot_fileNew,    QKeySequence::New);
    auto* actOpen   = mkAct(":/images/new_style/open.png",      tr("&Открыть"),
                          &McrWnd::slot_fileOpen,   QKeySequence::Open);
    auto* actSave   = mkAct(":/images/new_style/save.png",      tr("&Сохранить"),
                          &McrWnd::slot_fileSave,   QKeySequence::Save);
    auto* actSaveAs = mkAct(":/images/new_style/save as.png",   tr("Сохранить как"),
                            &McrWnd::slot_fileSaveAs, QKeySequence::SaveAs);
    auto* actRun    = mkAct(":/images/new_style/playback.png",        tr("&Пуск"),
                         &McrWnd::slot_run,         Qt::Key_F10);
    auto* actFind   = mkAct(":/images/new_style/search.png",       tr("&Найти…"),
                          &McrWnd::slot_find,        QKeySequence::Find);
    auto* actGoto   = mkAct(":/images/new_style/recognition of printed text.png",  tr("&Перейти к линии…"),
                          &McrWnd::slot_goToLine,
                          QKeySequence(Qt::CTRL | Qt::Key_G));
    auto* actClear  = mkAct(":/images/new_style/broomstick.png",  tr("Очистить лог"),
                           &McrWnd::slot_protClear);

    // ── Меню ─────────────────────────────────────────────────────────────────
    QMenu* fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addSeparator();
    fileMenu->addAction(actSave);
    fileMenu->addAction(actSaveAs);

    QMenu* runMenu = menuBar()->addMenu(tr("&Пуск"));
    runMenu->addAction(actRun);

    QMenu* editMenu = menuBar()->addMenu(tr("&Правка"));
    editMenu->addAction(actFind);
    editMenu->addAction(actGoto);
    editMenu->addSeparator();
    editMenu->addAction(actClear);

    // ── Тулбар ───────────────────────────────────────────────────────────────
    QToolBar* tb = addToolBar(tr("Main"));
    tb->setObjectName("McrWndMainToolBar"); // нужно для saveState/restoreState
    tb->setIconSize(QSize(16, 16));

    tb->addAction(actNew);
    tb->addAction(actOpen);
    tb->addAction(actSave);
    tb->addAction(actSaveAs);
    tb->addSeparator();
    tb->addAction(actRun);
    tb->addSeparator();
    tb->addAction(actFind);
    tb->addAction(actGoto);
    tb->addSeparator();
    tb->addAction(actClear);
}

void McrWnd::buildStatusBar()
{
    m_sbFile     = new QLabel(tr("Без названия"), this);
    m_sbPos      = new QLabel(tr("Ln 1, Col 1"), this);
    m_sbModified = new QLabel(this);

    // Фиксированные минимальные ширины, чтобы статусбар не прыгал
    m_sbPos->setMinimumWidth(110);
    m_sbPos->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_sbModified->setMinimumWidth(16);
    m_sbModified->setAlignment(Qt::AlignCenter);
    m_sbModified->setToolTip(tr("Несохраненные изменения"));

    statusBar()->addWidget(m_sbFile, 1);         // растягивается
    statusBar()->addPermanentWidget(m_sbPos);
    statusBar()->addPermanentWidget(m_sbModified);
}

void McrWnd::slot_updateStatusBar()
{
    const sptr_t pos  = m_editor->currentPos();
    const sptr_t line = m_editor->lineFromPosition(pos);
    const sptr_t col  = m_editor->column(pos);

    m_sbPos->setText(tr("Ln %1, Col %2").arg(line + 1).arg(col + 1));
    m_sbModified->setText(m_editor->isModified() ? QStringLiteral("*") : QString{});
}

void McrWnd::showEvent(QShowEvent* event)
{
    //базовый класс изменился
    QMainWindow::showEvent(event);

    if (!m_firstShow)
        return;
    m_firstShow = false;

    const auto btn = QMessageBox::question(
        this, tr("Macro Python"),
        tr("Загрузить пример скрипта?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (btn != QMessageBox::Yes)
        return;

    const QString examplePath =
        QCoreApplication::applicationDirPath() + "/../Data/py_example/rastr_events.py";

    m_editor->setFileInfo(QFileInfo(examplePath));
    if (SciHlpBase::RetVal::Ok != m_editor->loadFromFile()) {
        QMessageBox::warning(this, tr("Предупреждение"),
                             tr("Пример файла не найден:\n%1").arg(examplePath));
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
                   tr("Macro Editor — Python"),
                   tr("В макросе есть несохраненные изменения. Сохранить?"),
                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                   this);
    mb.setDefaultButton(QMessageBox::Yes);

    switch (mb.exec()) {
    case QMessageBox::Yes: {
        const bool ok = m_editor->getFileInfo().absoluteFilePath().isEmpty()
        ? slot_fileSaveAs()
        : slot_fileSave();
        return ok ? SavePromptResult::Saved : SavePromptResult::Cancelled;
    }
    case QMessageBox::No:
        return SavePromptResult::Discarded;
    default:
        return SavePromptResult::Cancelled;
    }
}

void McrWnd::closeEvent(QCloseEvent* event)
{
    const auto result = promptSaveIfModified();
    if (result == SavePromptResult::Cancelled) {
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
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
    if (promptSaveIfModified() == SavePromptResult::Cancelled)
        return;

    const QString path = QFileDialog::getOpenFileName(this, tr("Open file"));
    if (path.isEmpty())
        return;

    m_editor->setFileInfo(QFileInfo{path});
    if (SciHlpBase::RetVal::Ok != m_editor->loadFromFile()) {
        m_editor->setFileInfo(QFileInfo{});
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось загрузить файл:\n%1").arg(path));
    }
}

bool McrWnd::slot_fileSave()
{
    // Нет пути — делегируем в «Сохранить как»
    if (m_editor->getFileInfo().absoluteFilePath().isEmpty())
        return slot_fileSaveAs();

    if (SciHlpBase::RetVal::Ok != m_editor->saveToFile()) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось сохранить файл:\n%1")
                                  .arg(m_editor->getFileInfo().absoluteFilePath()));
        return false;
    }
    // снять флаг «*»
    slot_updateStatusBar();
    return true;
}

bool McrWnd::slot_fileSaveAs()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Сохраните файл как"));
    if (path.isEmpty())
        return false;

    if (SciHlpBase::RetVal::Ok != m_editor->setFileInfo(QFileInfo(path))) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удается задать путь к файлу:\n%1").arg(path));
        return false;
    }
    return slot_fileSave();
}

void McrWnd::slot_run()
{
    if (!m_pyHlp) { m_glodLogWidget->onRastrPrint("No PyHlp!\n"); return; }

    const QByteArray src = m_editor->getText(m_editor->textLength());
    const PyHlp::Result res = m_pyHlp->run(src.constData());
    if (res == PyHlp::Result::Ok) return;

    const QString errMsg = m_pyHlp->getErrorMessage().empty()
                               ? tr("Неизвестная ошибка")
                               : QString::fromStdString(m_pyHlp->getErrorMessage());
    const long errLine = m_pyHlp->getErrorLine();

    QMessageBox mb(QMessageBox::Critical, tr("Python error"), errMsg, {}, this);
    if (errLine > -1) {
        mb.setInformativeText(tr("Перейти к строке %1?").arg(errLine));
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
        this, tr("Перейти к строке"),
        tr("Номер строки (1..%1):").arg(total),
        1, 1, static_cast<int>(total), 1, &ok);
    if (ok)
        m_editor->gotoLine(line - 1);
}

void McrWnd::slot_find()
{
    // окно поиска уже создано и живо
    if (m_findDlg) {
        m_findDlg->raise();
        m_findDlg->activateWindow();
        return;
    }

    m_findDlg = new DlgFindRepl(this);
    // QPointer обнулится сам
    m_findDlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_findDlg, &DlgFindRepl::sig_find,
            this,      &McrWnd::slot_findByParams);
    m_findDlg->show();
}

void McrWnd::slot_findByParams(SciPyEditor::FindParams params)
{
    if (SciPyEditor::RetVal::Ok != m_editor->find(params))
        QMessageBox::information(this, tr("Найти"),
                                 tr("'%1' не найдена.").arg(params.m_text));
}

void McrWnd::slot_protClear()
{
    m_glodLogWidget->clear();
}

void McrWnd::slot_appendProtocol(const QString& qstr)
{
    m_glodLogWidget->onAppendText(qstr);
}

void McrWnd::slot_rastrLog(const _log_data& log_data){
    m_glodLogWidget->onRastrLog(log_data);
}

void McrWnd::slot_rastrPrint(const std::string& str_msg){
    m_glodLogWidget->onRastrPrint(str_msg);
}

void McrWnd::slot_fileInfoChanged(const QFileInfo& fi)
{
    const bool hasFile = !fi.absoluteFilePath().isEmpty();

    setWindowTitle(hasFile
                       ? tr("%1 — Macro Editor").arg(fi.fileName())
                       : tr("Macro Editor — Python"));

    m_sbFile->setText(hasFile ? fi.absoluteFilePath() : tr("Без названия"));
    m_sbFile->setToolTip(m_sbFile->text());

    slot_updateStatusBar();
}
