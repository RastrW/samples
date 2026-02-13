#include "fileManager.h"
#include "QAstra.h"
#include "Params.h"
#include "FormFileNew.h"
#include "formsaveall.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <spdlog/spdlog.h>

FileManager::FileManager(
    std::shared_ptr<QAstra> qastra,
    QWidget* parent
    )
    : QObject(parent)
    , m_qastra(qastra)
    , m_parentWidget(parent)
{
    assert(m_qastra != nullptr);
}

bool FileManager::newFile() {
    QStringList selectedTemplates;
    if (!showNewFileDialog(selectedTemplates)) {
        return false;
    }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    for (const QString& templateName : selectedTemplates) {
        const std::string templatePath = 
            Params::get_instance()->getDirSHABLON().absolutePath().toStdString() 
            + "//" + templateName.toStdString();
        
        IPlainRastrRetCode res = m_qastra->Load(eLoadCode::RG_REPL, "", templatePath);
        
        if (res == IPlainRastrRetCode::Ok) {
            spdlog::info("Template loaded: {}", templateName.toStdString());
        } else {
            spdlog::error("Failed to load template: {}", templateName.toStdString());
            QApplication::restoreOverrideCursor();
            emit fileLoadError(QString("Failed to load template: %1").arg(templateName));
            return false;
        }
    }
    
    QApplication::restoreOverrideCursor();
    
    m_currentFile.clear();
    
    return true;
}

int FileManager::openFiles() {
    QStringList selectedFiles;
    QString selectedFilter;

    if (!showOpenDialog(selectedFiles, selectedFilter)) {
        return 0;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    int successCount = 0;
    const QString noTemplateFilter = "No template (*)";
    const auto& templateExts = Params::get_instance()->getTemplateExts();

    // Загружаем КАЖДЫЙ выбранный файл
    for (const QString& filePath : selectedFiles) {
        spdlog::info("Loading file: {}", filePath.toStdString());

        bool loaded = false;
        QString usedTemplate;

        if (selectedFilter != noTemplateFilter) {
            // Ищем шаблон по расширению
            for (const auto& [templateName, templateExt] : templateExts) {
                if (filePath.endsWith(QString::fromStdString(templateExt))) {
                    const std::string fullTemplatePath =
                        Params::get_instance()->getDirSHABLON().absolutePath().toStdString()
                        + "//" + templateName + templateExt;

                    IPlainRastrRetCode res = m_qastra->Load(
                        eLoadCode::RG_REPL,
                        filePath.toStdString(),
                        fullTemplatePath
                        );

                    if (res == IPlainRastrRetCode::Ok) {
                        usedTemplate = QString::fromStdString(fullTemplatePath);
                        loaded = true;
                        spdlog::info("Loaded with template: {}", fullTemplatePath);
                        break;
                    } else {
                        spdlog::error("Load failed (code: {})", static_cast<int>(res));
                    }
                }
            }

            if (!loaded) {
                spdlog::error("No matching template found for: {}", filePath.toStdString());
            }
        } else {
            // Загрузка БЕЗ шаблона
            IPlainRastrRetCode res = m_qastra->Load(
                eLoadCode::RG_REPL,
                filePath.toStdString(),
                ""
                );

            if (res == IPlainRastrRetCode::Ok) {
                loaded = true;
                spdlog::info("Loaded without template");
            } else {
                spdlog::error("Load failed (code: {})", static_cast<int>(res));
            }
        }

        if (loaded) {
            // setCurrentFile добавляет в m_loadedFiles!
            setCurrentFile(filePath, usedTemplate);
            successCount++;
        } else {
            emit fileLoadError(QString("Failed to load: %1").arg(filePath));
        }
    }

    QApplication::restoreOverrideCursor();

    if (successCount > 0) {
        emit filesOpened(successCount);
        spdlog::info("Successfully loaded {} files", successCount);
    }

    return successCount;
}

bool FileManager::openFile(const QString& filePath, const QString& templatePath) {
    if (filePath.isEmpty()) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    IPlainRastrRetCode res = m_qastra->Load(
        eLoadCode::RG_REPL,
        filePath.toStdString(),
        templatePath.toStdString()
        );

    QApplication::restoreOverrideCursor();

    if (res != IPlainRastrRetCode::Ok) {
        QString error = QString("Failed to load: %1 (code: %2)")
        .arg(filePath)
            .arg(static_cast<int>(res));
        spdlog::error("{}", error.toStdString());
        emit fileLoadError(error);
        return false;
    }

    setCurrentFile(filePath, templatePath);
    spdlog::info("File loaded: {}", filePath.toStdString());

    return true;
}

bool FileManager::save() {
    if (m_currentFile.isEmpty()) {
        return saveAs();
    }

    m_qastra->Save(m_currentFile.toStdString().c_str(), "");

    spdlog::info("File saved: {}", m_currentFile.toStdString());
    emit fileSaved(m_currentFile);

    return true;
}

bool FileManager::saveAs() {
    QString filePath = showSaveDialog();
    if (filePath.isEmpty()) {
        return false;
    }

    QString templatePath = findTemplateByExtension(filePath);

    const std::string fullTemplatePath =
        Params::get_instance()->getDirSHABLON().absolutePath().toStdString()
        + "/" + templatePath.toStdString();

    m_qastra->Save(filePath.toStdString(), fullTemplatePath.c_str());

    setCurrentFile(filePath, templatePath);

    spdlog::info("File saved as: {}", filePath.toStdString());
    emit fileSaved(filePath);

    return true;
}

bool FileManager::saveAll() {
    if (m_loadedFiles.isEmpty()) {
        spdlog::warn("No files loaded to save");
        return false;
    }

    // Передаём ВСЮ карту загруженных файлов
    formsaveall* dialog = new formsaveall(
        m_qastra.get(),
        m_loadedFiles,
        m_parentWidget
        );

    dialog->show();
    return true;
}

void FileManager::openRecentFile(const QString& fileAndTemplate) {
    QStringList parts = fileAndTemplate.split(" <");

    QString file = parts[0];
    QString templatePath;

    if (parts.size() > 1) {
        templatePath = parts[1];
        templatePath.chop(1); // Remove '>'
    }

    openFile(file, templatePath);
}

void FileManager::setCurrentFile(const QString& fileName, const QString& templatePath) {
    QFileInfo fileInfo(fileName);

    m_currentFile = fileName;
    m_currentDir = fileInfo.absolutePath();

    m_loadedFiles[templatePath] = fileName;

    addToRecentFiles(fileName, templatePath);

    emit currentFileChanged(fileName);
    emit fileOpened(fileName);
}

void FileManager::addToRecentFiles(const QString& filePath, const QString& templatePath) {
    QSettings settings;

    QString fileAndTemplate = filePath;
    if (!templatePath.isEmpty()) {
        fileAndTemplate += " <" + templatePath + ">";
    }

    QStringList files = settings.value(RecentFilesKey).toStringList();
    files.removeAll(fileAndTemplate);
    files.prepend(fileAndTemplate);

    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    settings.setValue(RecentFilesKey, files);
    emit recentFilesChanged();
}

QStringList FileManager::getRecentFiles() const {
    QSettings settings;
    return settings.value(RecentFilesKey).toStringList();
}

bool FileManager::showOpenDialog(QStringList& selectedFiles, QString& selectedFilter) {
    QFileDialog dialog(m_parentWidget, tr("Open Rastr files"));
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setViewMode(QFileDialog::Detail);

    dialog.setFileMode(QFileDialog::ExistingFiles);

    QString filter = buildFileFilter();
    dialog.setNameFilter(filter);

    if (!m_currentDir.isEmpty()) {
        dialog.setDirectory(m_currentDir);
    }

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    selectedFiles = dialog.selectedFiles();
    selectedFilter = dialog.selectedNameFilter();

    return !selectedFiles.isEmpty();
}

QString FileManager::showSaveDialog() {
    QFileDialog dialog(m_parentWidget, tr("Save Rastr file"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::AnyFile);

    QString filter = buildFileFilter();
    dialog.setNameFilter(filter);
    dialog.selectNameFilter("режим (*.rg2)");

    if (!m_currentDir.isEmpty()) {
        dialog.setDirectory(m_currentDir);
    }

    if (dialog.exec() != QDialog::Accepted) {
        return QString();
    }

    return dialog.selectedFiles().first();
}

QString FileManager::buildFileFilter() const {
    QString filter;

    // Known types (все расширения)
    filter += "Known types(";
    const auto& extensions = Params::get_instance()->getTemplateExts();
    for (const auto& [name, ext] : extensions) {
        filter += QString("*%1 ").arg(QString::fromStdString(ext));
    }
    filter += ");;";

    // No template - позволяет загрузку без шаблона
    filter += "No template (*);;";

    // Конкретные типы
    filter += "режим (*.rg2);;";
    filter += "poisk (*.os);;";
    filter += "графика (*.grf)";

    return filter;
}

QString FileManager::findTemplateByExtension(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString ext = "." + fileInfo.suffix();

    const auto& extensions = Params::get_instance()->getTemplateExts();
    for (const auto& [templateName, templateExt] : extensions) {
        if (QString::fromStdString(templateExt) == ext) {
            return QString::fromStdString(templateName + templateExt);
        }
    }

    return QString();
}

bool FileManager::showNewFileDialog(QStringList& selectedTemplates) {
    FormFileNew* dialog = new FormFileNew(m_parentWidget);

    if (dialog->exec() != QDialog::Accepted) {
        return false;
    }

    const auto& checked = dialog->getCheckedTemplateNames();
    for (const auto& templateName : checked) {
        selectedTemplates.append(QString::fromStdString(templateName));
    }

    return !selectedTemplates.isEmpty();
}
