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
    if (m_loadedFiles.empty())
        return saveAs();

    bool allOk = true;
    for (const auto& [templateKey, filePath] : m_loadedFiles) {
        if (filePath.isEmpty())
            continue;

        const std::string sfile  = filePath.toStdString();
        const std::string sshabl = templateKey.isEmpty()
                                       ? ""
                                       : RastrParameters::get_instance()
                                             ->getDirSHABLON()
                                             .filePath(templateKey)
                                             .toStdString();

        if (!m_fileOps->Save(sfile, sshabl)) {
            const QString err = QString("Save failed: %1 — %2")
                                    .arg(filePath,
                                         QString::fromStdString(m_fileOps->lastError()));
            spdlog::error("{}", err.toStdString());
            emit sig_fileLoadError(err);
            allOk = false;
        } else {
            spdlog::info("File saved: {}", sfile);
            emit sig_fileSaved(filePath);
        }
    }
    return allOk;
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
    dialog->exec();
    return true;
}

void FileManager::setCurrentFile(const QString& fileName,
                                 const QString& templatePath) {
    const QString templateKey  = QFileInfo(templatePath).fileName();
    m_loadedFiles[templateKey] = fileName;

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
    const QString templateKey  = QFileInfo(templatePath).fileName();
    m_loadedFiles[templateKey] = fileName;
}

bool FileManager::showOpenDialog(QStringList& selectedFiles, QString& selectedFilter){

    QFileDialog dlg(m_parentWidget, tr("Open Rastr files"));
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setNameFilter(buildFileFilter());

    const QString dir = currentDirectory();
    if (!dir.isEmpty())
        dlg.setDirectory(dir);

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
    const QString dir = currentDirectory();
    if (!dir.isEmpty())
        dlg.setDirectory(dir);

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

QString FileManager::currentDirectory() const {
    if (m_loadedFiles.empty())
        return {};
    // Берём путь любого файла из map — все в одной директории как правило
    return QFileInfo(m_loadedFiles.begin()->second).absolutePath();
}