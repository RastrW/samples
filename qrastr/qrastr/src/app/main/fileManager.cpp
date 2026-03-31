#include "fileManager.h"
#include "qastra.h"
#include "params.h"
#include "files/fileNewDialog.h"
#include "files/saveAllDialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <spdlog/spdlog.h>
#include "cursor_guard.h"

FileManager::FileManager(std::shared_ptr<QAstra> qastra, QWidget* parent)
    : QObject(parent)
    , m_qastra(qastra)
    , m_parentWidget(parent)
{
    assert(m_qastra != nullptr);
}

bool FileManager::newFile() {
    QStringList selectedTemplates;
    if (!showNewFileDialog(selectedTemplates))
        return false;

    {
        CursorGuard guard;
        for (const QString& templateName : selectedTemplates) {
            const std::string path =
                Params::get_instance()->getDirSHABLON().absolutePath().toStdString()
                + "//" + templateName.toStdString();
            m_qastra->Load(eLoadCode::RG_REPL, "", path);
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
    const auto& templateExts = Params::get_instance()->getTemplateExts();
    //Загружаем каждый выбранный файл
    for (const QString& file : selectedFiles) {
        spdlog::info("Try load file: {}", file.toStdString());

        if (selectedFilter == noTemplate) {
            // Загрузка без шаблона
            IPlainRastrRetCode res =
                m_qastra->Load(eLoadCode::RG_REPL, file.toStdString(), "");
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
                const fs::path shablon =
                    fs::path(Params::get_instance()->getDirSHABLON()
                                 .absolutePath().toStdString())
                    / (name + ext);

                IPlainRastrRetCode res = m_qastra->Load(
                    eLoadCode::RG_REPL,
                    file.toStdString(),
                    shablon.string());

                if (res == IPlainRastrRetCode::Ok) {
                    setCurrentFile(file, QString::fromStdString(shablon.string()));
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
        res = m_qastra->Load(
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
    
    m_qastra->Save(m_currentFile.toStdString().c_str(), "");
    
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
            Params::get_instance()->getDirSHABLON().absolutePath().toStdString() 
            + "/" + templatePath.toStdString();
    }
    
    m_qastra->Save(
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
        m_qastra.get(),
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
    QSettings settings;
    
    QString _fileshabl = filePath;
    if (!templatePath.isEmpty()) {
        _fileshabl.append(" <").append(templatePath).append(">");
    }
    
    QStringList files = settings.value(m_recentFilesKey).toStringList();
    files.removeAll(_fileshabl);
    files.prepend(_fileshabl);

    auto* const p_params = Params::get_instance();
    while (files.size() > p_params->getMaxRecentFiles()) {
        files.removeLast();
    }
    
    settings.setValue(m_recentFilesKey, files);
    
    emit recentFilesChanged();
}

QStringList FileManager::getRecentFiles() const {
    QSettings settings;
    return settings.value(m_recentFilesKey).toStringList();
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

void FileManager::openRecentFile(const QString& fileAndTemplate) {
    QStringList qslist = fileAndTemplate.split("<");
    
    std::string file = qslist[0].trimmed().toStdString();
    std::string shabl = "";
    
    if (qslist.size() > 1) {
        shabl = qslist[1].toStdString();
        shabl.erase(shabl.end() - 1);  // Удаляем '>'
    }
    
    m_qastra->Load(eLoadCode::RG_REPL, file, shabl);
    setCurrentFile(qslist[0], QString::fromStdString(shabl));
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
    const Params::_v_template_exts v_template_ext = Params::get_instance()->getTemplateExts();
    for (const Params::_v_template_exts::value_type& template_ext : v_template_ext) {
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
    const Params::_v_template_exts v_template_ext = Params::get_instance()->getTemplateExts();
    
    for (const Params::_v_template_exts::value_type& template_ext : v_template_ext) {
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
