#include "fileManager.h"
#include "qastra.h"
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

    {
        CursorGuard guard;
        for (const QString& templateName : selectedTemplates) {
            const std::string path =
                RastrParameters::get_instance()->getDirSHABLON().absolutePath().toStdString()
                + "//" + templateName.toStdString();
            m_fileOps->Load(eLoadCode::RG_REPL, "", path);
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

    CursorGuard guard;  // ← охватывает весь цикл, включая возможные исключения

    int successCount = 0;
    const QString noTemplate = "No template (*)";
    // Получаем список шаблонов из Params
    const auto& templateExts = RastrParameters::get_instance()->getTemplateExts();
    //Загружаем каждый выбранный файл
    for (const QString& file : selectedFiles) {
        spdlog::info("Try load file: {}", file.toStdString());

        if (selectedFilter == noTemplate) {
            // Загрузка без шаблона
            IPlainRastrRetCode res =
                m_fileOps->Load(eLoadCode::RG_REPL, file.toStdString(), "");
            if (res == IPlainRastrRetCode::Ok) {
                setCurrentFile(file, "");
                ++successCount;
            } else {
                emit fileLoadError(QString("Failed to load: %1").arg(file));
            }
            continue;
        }

        // Загрузка с шаблоном — ищем по расширению
        bool templateFound = false;
        for (const auto& [name, ext] : templateExts) {
            if (!file.endsWith(QString::fromStdString(ext)))
                continue;

            templateFound = true;
            try {
                const QString shablon =
                    RastrParameters::get_instance()->getDirSHABLON()
                        .filePath(QString::fromStdString(name + ext));

                IPlainRastrRetCode res = m_fileOps->Load(
                    eLoadCode::RG_REPL,
                    file.toStdString(),
                    shablon.toStdString());

                if (res == IPlainRastrRetCode::Ok){
                    setCurrentFile(file, shablon);
                    ++successCount;
                } else {
                    emit fileLoadError(QString("Failed to load: %1").arg(file));
                }
            } catch (const std::exception& ex) {
                spdlog::error("Exception loading file {}: {}", file.toStdString(), ex.what());
                QMessageBox::critical(m_parentWidget, tr("Ошибка"),
                                      QString("Ошибка при чтении файла: %1").arg(file));
            }
            break;
        }

        if (!templateFound) {
            spdlog::error("Template not found for: {}", file.toStdString());
            emit fileLoadError(QString("Template not found for: %1").arg(file));
        }
    }

    if (successCount > 0)
        emit filesOpened(successCount);

    return successCount;
}

bool FileManager::openFile(const QString& filePath, const QString& templatePath) {
    if (filePath.isEmpty())
        return false;

    IPlainRastrRetCode res;
    {
        CursorGuard guard;
        res = m_fileOps->Load(
            eLoadCode::RG_REPL,
            filePath.toStdString(),
            templatePath.toStdString());
    }

    if (res != IPlainRastrRetCode::Ok) {
        const QString error =
            QString("Failed to load file: %1 (code: %2)")
                .arg(filePath)
                .arg(static_cast<int>(res));
        spdlog::error("{}", error.toStdString());
        emit fileLoadError(error);
        return false;
    }

    setCurrentFile(filePath, templatePath);
    spdlog::info("File loaded successfully: {}", filePath.toStdString());
    return true;
}

bool FileManager::save() {
    if (m_currentFile.isEmpty()) {
        return saveAs();
    }
    
    m_fileOps->Save(m_currentFile.toStdString().c_str(), "");
    
    std::string str_msg = fmt::format("{}: {}", "Сохранен файл", m_currentFile.toStdString());
    emit fileSaved(m_currentFile);
    
    return true;
}

bool FileManager::saveAs() {
    QString filePath = showSaveDialog();
    if (filePath.isEmpty()) {
        return false;
    }
    
    // Определяем шаблон из выбранного фильтра
    QString templatePath = findTemplateByExtension(filePath);
    
    std::string str_path_to_shablon;
    if (!templatePath.isEmpty()) {
        str_path_to_shablon = 
            RastrParameters::get_instance()->getDirSHABLON().absolutePath().toStdString()
            + "/" + templatePath.toStdString();
    }
    
    m_fileOps->Save(
        filePath.toStdString(),
        str_path_to_shablon.c_str()
    );
    
    setCurrentFile(filePath, templatePath);
    
    emit fileSaved(filePath);
    
    return true;
}

bool FileManager::saveAll() {
    if (m_loadedFiles.empty()) {
        spdlog::warn("No files loaded to save");
        return false;
    }

    SaveAllDialog* fsaveall = new SaveAllDialog(
        m_fileOps,
        m_loadedFiles,
        m_parentWidget
    );

    fsaveall->setAttribute(Qt::WA_DeleteOnClose);

    fsaveall->show();
    
    return true;
}

void FileManager::setCurrentFile(const QString& fileName,
                                 const QString& templatePath) {
    QFileInfo fileInfo(fileName);
    
    // Обновляем текущий файл и директорию
    m_currentFile = fileName;
    m_currentDir = fileInfo.absolutePath();
    m_loadedFiles[templatePath] = fileName;
    
    // Добавляем в недавние
    addToRecentFiles(fileName, templatePath);
    
    emit currentFileChanged(fileName);
    emit fileOpened(fileName);
}

void FileManager::addToRecentFiles(const QString& filePath,
                                   const QString& templatePath) {
    QSettings s;
    int maxRecent = s.value(SK::Files::maxRecentFiles,
                            SK::Files::defMaxRecent).toInt();

    auto entries = loadRecentFiles();

    // Убираем дубликат по пути файла
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [&](const RecentFileEntry& e) { return e.file == filePath; }),
                  entries.end());

    entries.prepend({filePath, templatePath});

    while (entries.size() > maxRecent)
        entries.removeLast();

    saveRecentFiles(entries);
}

void FileManager::saveRecentFiles(const QList<RecentFileEntry>& entries) {
    QSettings s;
    s.remove(SK::Files::recentFiles);
    s.beginWriteArray(SK::Files::recentFiles);
    for (int i = 0; i < entries.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("file", entries[i].file);
        s.setValue("tmpl", entries[i].tmpl);
    }
    s.endArray();
}

QList<RecentFileEntry> FileManager::loadRecentFiles() const {
    QSettings s;
    int n = s.beginReadArray(SK::Files::recentFiles);
    QList<RecentFileEntry> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        result.append({
            s.value("file").toString(),
            s.value("tmpl").toString()
        });
    }
    s.endArray();
    return result;
}

QList<RecentFileEntry> FileManager::getRecentFiles() const {
    return loadRecentFiles();
}

void FileManager::openRecentFile(const QString& filePath,
                                 const QString& templatePath) {
    m_fileOps->Load(eLoadCode::RG_REPL,
                   filePath.toStdString(),
                   templatePath.toStdString());
    setCurrentFile(filePath, templatePath);
}

void FileManager::registerStartupFile(const QString& fileName,
                                      const QString& templatePath) {
    // Только в карту — без addToRecentFiles
    m_loadedFiles[templatePath] = fileName;
    if (m_currentFile.isEmpty()) {
        m_currentFile = fileName;
        m_currentDir  = QFileInfo(fileName).absolutePath();
    }
}

bool FileManager::showOpenDialog(QStringList& selectedFiles, QString& selectedFilter)
{
    QFileDialog fileDlg(m_parentWidget, tr("Open Rastr files"));

    fileDlg.setFileMode(QFileDialog::ExistingFiles);
    fileDlg.setNameFilter(buildFileFilter());

    if (!m_currentDir.isEmpty()){
        fileDlg.setDirectory(m_currentDir);
    }

    if (fileDlg.exec() != QDialog::Accepted){
        return false;
    }

    selectedFiles = fileDlg.selectedFiles();
    selectedFilter = fileDlg.selectedNameFilter();

    return !selectedFiles.isEmpty();
}

QString FileManager::showSaveDialog() {

    QFileDialog fileDlg(m_parentWidget, tr("Save Rastr file"));

    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setViewMode(QFileDialog::Detail);

    fileDlg.setNameFilter(buildFileFilter());
    fileDlg.selectNameFilter("режим (*.rg2)");
    
    fileDlg.setFileMode(QFileDialog::AnyFile);
    
    if (!m_currentDir.isEmpty()) {
        fileDlg.setDirectory(m_currentDir);
    }

    if (fileDlg.exec() != QDialog::Accepted) {
        return QString();
    }
    
    return fileDlg.selectedFiles()[0];
}

QString FileManager::buildFileFilter() const {
    QString qstr_filter;
    
    // Known types (все расширения вместе)
    qstr_filter += "Known types(";
    const RastrParameters::_v_template_exts v_template_ext = RastrParameters::get_instance()->getTemplateExts();
    for (const RastrParameters::_v_template_exts::value_type& template_ext : v_template_ext) {
        qstr_filter += QString("*%1 ").arg(QString::fromStdString(template_ext.second));
    }
    qstr_filter += ");;";
    
    // No template - важен для выбора загрузки без шаблона
    const QString qstr_filter_no_template = "No template (*)";
    qstr_filter += qstr_filter_no_template;
    
    // Конкретные типы
    qstr_filter += ";;режим (*.rg2)";
    qstr_filter += ";;poisk (*.os)";
    qstr_filter += ";;графика (*.grf)";
    
    return qstr_filter;
}

QString FileManager::findTemplateByExtension(const QString& filePath) const {
    const RastrParameters::_v_template_exts v_template_ext =
        RastrParameters::get_instance()->getTemplateExts();
    
    for (const RastrParameters::_v_template_exts::value_type& template_ext : v_template_ext) {
        if (filePath.endsWith(QString::fromStdString(template_ext.second))) {
            return QString::fromStdString(template_ext.first + template_ext.second);
        }
    }
    
    return QString();
}

bool FileManager::showNewFileDialog(QStringList& selectedTemplates) {
    // Создаём на стеке — время жизни совпадает с областью видимости функции.
    // WA_DeleteOnClose здесь НЕ нужен и ОПАСЕН для модальных диалогов.
    FileNewDialog dialog(m_parentWidget);

    if (QDialog::Accepted != dialog.exec())
        return false;

    // dialog ещё жив — getCheckedTemplateNames() безопасен
    for (const auto& name : dialog.getCheckedTemplateNames())
        selectedTemplates.append(QString::fromStdString(name));

    return !selectedTemplates.isEmpty();
}
