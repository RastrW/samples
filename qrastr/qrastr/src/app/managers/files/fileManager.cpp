#include "fileManager.h"
#include "rastrParameters.h"
#include "files/fileNewDialog.h"
#include "files/saveAllDialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <spdlog/spdlog.h>
#include "cursor_guard.h"
#include "settingsKeys.h"
#include "files/IFileOperations.h"

FileManager::FileManager(std::shared_ptr<IFileOperations> fileOps, QWidget* parent)
    : QObject(parent)
    , m_fileOps(fileOps)
    , m_parentWidget(parent)
{

    const auto& startFiles = RastrParameters::get_instance()->getStartLoadFileTemplates();
    for (const auto& [file, tmpl] : startFiles) {
        // Добавляем в карту БЕЗ добавления в "последние"
        registerStartupFile(QString::fromStdString(file),
                            QString::fromStdString(tmpl));
    }

}

bool FileManager::newFile() {
    QStringList selectedTemplates;
    if (!showNewFileDialog(selectedTemplates))
        return false;

    const QDir shabDir = RastrParameters::get_instance()->getDirSHABLON();
    CursorGuard guard;
    for (const QString& templateName : selectedTemplates){
        if (m_fileOps->Load(eLoadCode::RG_REPL, "",
                            shabDir.filePath(templateName).toStdString())){
            return false;
        }
    }
    // Новый файл - сбрасываем текущий путь
    m_currentFile.clear();
    return true;
}

int FileManager::openFiles() {
    QStringList selectedFiles;
    QString     selectedFilter;

    if (!showOpenDialog(selectedFiles, selectedFilter))
        return 0;

    CursorGuard guard;
    int successCount = 0;
    // Получаем список шаблонов из Params
    const auto& templateExts = RastrParameters::get_instance()->getTemplateExts();

    for (const QString& file : selectedFiles) {
        spdlog::info("Try load file: {}", file.toStdString());

        if (selectedFilter == QLatin1String(k_noTemplateFilter)) {
            // Загрузка без шаблона
            if (m_fileOps->Load(eLoadCode::RG_REPL, file.toStdString(), "")) {
                setCurrentFile(file, {});
                ++successCount;
            } else {
                emit sig_fileLoadError(
                    QString("Failed to load: %1 — %2")
                        .arg(file, QString::fromStdString(m_fileOps->lastError())));
            }
            continue;
        }
        // Загрузка с шаблоном — ищем по расширению
        bool templateFound = false;
        for (const auto& [name, ext] : templateExts) {
            if (!file.endsWith(QString::fromStdString(ext)))
                continue;

            templateFound = true;
            const QString shablon =
                RastrParameters::get_instance()->getDirSHABLON()
                    .filePath(QString::fromStdString(name + ext));

            if (m_fileOps->Load(eLoadCode::RG_REPL,
                                file.toStdString(),
                                shablon.toStdString())) {
                setCurrentFile(file, shablon);
                ++successCount;
            } else {
                emit sig_fileLoadError(
                    QString("Failed to load: %1 — %2")
                        .arg(file, QString::fromStdString(m_fileOps->lastError())));
            }
            break;
        }

        if (!templateFound) {
            spdlog::error("Template not found for: {}", file.toStdString());
            emit sig_fileLoadError(QString("Template not found for: %1").arg(file));
        }
    }

    if (successCount > 0)
        emit sig_filesOpened(successCount);

    return successCount;
}

bool FileManager::openFile(const QString& filePath, const QString& templatePath) {
    if (filePath.isEmpty())
        return false;

    bool ok;
    {
        CursorGuard guard;
        ok = m_fileOps->Load(eLoadCode::RG_REPL,
                             filePath.toStdString(),
                             templatePath.toStdString());
    }

    if (!ok) {
        const QString err = QString("Failed to load: %1 — %2")
                                .arg(filePath, QString::fromStdString(m_fileOps->lastError()));
        spdlog::error("{}", err.toStdString());
        emit sig_fileLoadError(err);
        return false;
    }

    setCurrentFile(filePath, templatePath);
    spdlog::info("File loaded: {}", filePath.toStdString());
    return true;
}

bool FileManager::save() {
    if (m_currentFile.isEmpty())
        return saveAs();

    const std::string stdPath = m_currentFile.toStdString();
    if (!m_fileOps->Save(stdPath, "")) {
        const QString err = QString("Save failed: %1 — %2")
                                .arg(m_currentFile, QString::fromStdString(m_fileOps->lastError()));
        spdlog::error("{}", err.toStdString());
        emit sig_fileLoadError(err);
        return false;
    }

    spdlog::info("File saved: {}", stdPath);
    emit sig_fileSaved(m_currentFile);
    return true;
}

bool FileManager::saveAs() {
    const QString filePath = showSaveDialog();
    if (filePath.isEmpty())
        return false;
    // Определяем шаблон из выбранного фильтра
    const QString templatePath = findTemplateByExtension(filePath);

    std::string templateStd;
    if (!templatePath.isEmpty()) {
        templateStd =
            RastrParameters::get_instance()->getDirSHABLON().absolutePath().toStdString()
            + "/" + templatePath.toStdString();
    }

    if (!m_fileOps->Save(filePath.toStdString(), templateStd)) {
        const QString err = QString("SaveAs failed: %1 — %2")
                                .arg(filePath, QString::fromStdString(m_fileOps->lastError()));
        spdlog::error("{}", err.toStdString());
        emit sig_fileLoadError(err);
        return false;
    }

    setCurrentFile(filePath, templatePath);
    emit sig_fileSaved(filePath);
    return true;
}

bool FileManager::saveAll() {
    if (m_loadedFiles.empty()) {
        spdlog::warn("No files loaded to save");
        return false;
    }

    auto* dialog = new SaveAllDialog(m_fileOps, m_loadedFiles, m_parentWidget);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    return true;
}

void FileManager::setCurrentFile(const QString& fileName,
                                 const QString& templatePath) {
    // Обновляем текущий файл и директорию
    m_currentFile               = fileName;
    m_currentDir                = QFileInfo(fileName).absolutePath();
    m_loadedFiles[templatePath] = fileName;
    // Добавляем в недавние
    m_recentFiles.add(fileName, templatePath);
    emit sig_currentFileChanged(fileName);
    emit sig_fileOpened(fileName);
}

QList<RecentFileEntry> FileManager::getRecentFiles() const {
    return m_recentFiles.load();
}

void FileManager::openRecentFile(const QString& filePath,
                                 const QString& templatePath) {
    if (!m_fileOps->Load(eLoadCode::RG_REPL,
                         filePath.toStdString(),
                         templatePath.toStdString()))
    {
        emit sig_fileLoadError(
            QString("Failed to open recent: %1 — %2")
                .arg(filePath, QString::fromStdString(m_fileOps->lastError())));
        return;
    }
    setCurrentFile(filePath, templatePath);
}

void FileManager::registerStartupFile(const QString& fileName,
                                      const QString& templatePath) {
    m_loadedFiles[templatePath] = fileName;
    if (m_currentFile.isEmpty()) {
        m_currentFile = fileName;
        m_currentDir  = QFileInfo(fileName).absolutePath();
    }
}

bool FileManager::showOpenDialog(QStringList& selectedFiles, QString& selectedFilter){

    QFileDialog dlg(m_parentWidget, tr("Open Rastr files"));
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilter(buildFileFilter());
    if (!m_currentDir.isEmpty())
        dlg.setDirectory(m_currentDir);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    selectedFiles  = dlg.selectedFiles();
    selectedFilter = dlg.selectedNameFilter();
    return !selectedFiles.isEmpty();
}

QString FileManager::showSaveDialog() {  
    QFileDialog dlg(m_parentWidget, tr("Save Rastr file"));
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setViewMode(QFileDialog::Detail);
    dlg.setNameFilter(buildFileFilter());
    dlg.selectNameFilter("режим (*.rg2)");
    dlg.setFileMode(QFileDialog::AnyFile);
    if (!m_currentDir.isEmpty())
        dlg.setDirectory(m_currentDir);

    if (dlg.exec() != QDialog::Accepted)
        return {};

    return dlg.selectedFiles().value(0);
}

QString FileManager::buildFileFilter() const {
    const auto& templateExts = RastrParameters::get_instance()->getTemplateExts();

    QStringList exts;
    exts.reserve(static_cast<int>(templateExts.size()));
    for (const auto& [name, ext] : templateExts)
        exts << QString("*%1").arg(QString::fromStdString(ext));
    // Known types (все расширения вместе)
    QStringList filters;
    filters << QString("Known types(%1)").arg(exts.join(' '));
    // No template - важен для выбора загрузки без шаблона
    filters << QLatin1String(k_noTemplateFilter);
    // Конкретные типы
    filters << "режим (*.rg2)";
    filters << "poisk (*.os)";
    filters << "графика (*.grf)";

    return filters.join(";;");
}

QString FileManager::findTemplateByExtension(const QString& filePath) const {
    const auto& templateExts = RastrParameters::get_instance()->getTemplateExts();
    for (const auto& [name, ext] : templateExts) {
        if (filePath.endsWith(QString::fromStdString(ext)))
            return QString::fromStdString(name + ext);
    }
    return {};
}

bool FileManager::showNewFileDialog(QStringList& selectedTemplates) {
    FileNewDialog dialog(m_parentWidget);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    for (const auto& name : dialog.getCheckedTemplateNames())
        selectedTemplates.append(QString::fromStdString(name));

    return !selectedTemplates.isEmpty();
}
