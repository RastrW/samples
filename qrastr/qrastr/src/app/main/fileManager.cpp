#include "fileManager.h"
#include "qastra.h"
#include "params.h"
#include "formfilenew.h"
#include "formsaveall.h"

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <spdlog/spdlog.h>

FileManager::FileManager(std::shared_ptr<QAstra> qastra, QWidget* parent)
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
        const std::string str_path_to_shablon = 
            Params::get_instance()->getDirSHABLON().absolutePath().toStdString() 
            + "//" + templateName.toStdString();
        
        m_qastra->Load(eLoadCode::RG_REPL, "", str_path_to_shablon);
    }
    
    QApplication::restoreOverrideCursor();
    
    // Новый файл - сбрасываем текущий путь
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
    const QString qstr_filter_no_template = "No template (*)";
    
    // Получаем список шаблонов из Params
    const Params::_v_template_exts v_template_ext =
        Params::get_instance()->getTemplateExts();
    
    //Загружаем каждый выбранный файл
    for (const QString& rfile : selectedFiles) {
        spdlog::info("Try load file: {}", rfile.toStdString());
        
        if (qstr_filter_no_template != selectedFilter) {
            // Пользователь выбрал фильтр С шаблоном
            bool bl_find_template = false;
            
            for (const Params::_v_template_exts::value_type& template_ext : v_template_ext) {
                if (rfile.endsWith(QString::fromStdString(template_ext.second))) {
                    bl_find_template = true;
                    
                    const std::string str_path_to_shablon = 
                        Params::get_instance()->getDirSHABLON().absolutePath().toStdString() 
                        + "//" + template_ext.first + template_ext.second;
                    
                    IPlainRastrRetCode res = m_qastra->Load(
                        eLoadCode::RG_REPL,
                        rfile.toStdString(),
                        str_path_to_shablon
                    );
                    
                    if (res == IPlainRastrRetCode::Ok) {
                        setCurrentFile(rfile, QString::fromStdString(str_path_to_shablon));

                        // Запоминаем для сохранения в appsettings
                        Params::get_instance()->addStartLoadFileTemplate(
                            rfile.toStdString(),
                            str_path_to_shablon);

                        spdlog::info("File loaded {}", rfile.toStdString());
                        successCount++;
                    } else {
                        spdlog::info("Error File load result = {}", static_cast<int>(res));
                        emit fileLoadError(QString("Failed to load: %1").arg(rfile));
                    }
                    
                    break;
                }
            }

            if (!bl_find_template) {
                spdlog::error("Template not found for: {}", rfile.toStdString());
                emit fileLoadError(QString("Template not found for: %1").arg(rfile));
            }
        } else {
            // Загрузка БЕЗ шаблона
            IPlainRastrRetCode res = m_qastra->Load(
                eLoadCode::RG_REPL,
                rfile.toStdString(),
                ""
            );
            
            if (res == IPlainRastrRetCode::Ok) {
                setCurrentFile(rfile, "");
                spdlog::info("File loaded {}", rfile.toStdString());
                successCount++;
            } else {
                spdlog::info("Error File load result = {}", static_cast<int>(res));
                emit fileLoadError(QString("Failed to load: %1").arg(rfile));
            }
        }
    }
    
    QApplication::restoreOverrideCursor();
    
    if (successCount > 0) {
        emit filesOpened(successCount);
    }
    
    return successCount;
}

bool FileManager::openFile(const QString& filePath,
                           const QString& templatePath) {
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
        QString error = QString("Failed to load file: %1 (code: %2)")
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
    if (m_loadedFiles.isEmpty()) {
        spdlog::warn("No files loaded to save");
        return false;
    }

    formsaveall* fsaveall = new formsaveall(
        m_qastra.get(),
        m_loadedFiles,
        m_parentWidget
    );
    
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
    
    while (files.size() > m_maxRecentFiles) {
        files.removeLast();
    }
    
    settings.setValue(m_recentFilesKey, files);
    
    emit recentFilesChanged();
}

QStringList FileManager::getRecentFiles() const {
    QSettings settings;
    return settings.value(m_recentFilesKey).toStringList();
}

void FileManager::openRecentFile(const QString& fileAndTemplate) {
    QStringList qslist = fileAndTemplate.split("<");
    
    std::string file = qslist[0].toStdString();
    std::string shabl = "";
    
    if (qslist.size() > 1) {
        shabl = qslist[1].toStdString();
        shabl.erase(shabl.end() - 1);  // Удаляем '>'
    }
    
    m_qastra->Load(eLoadCode::RG_REPL, file, shabl);
    setCurrentFile(qslist[0], QString::fromStdString(shabl));
}

bool FileManager::showOpenDialog(QStringList& selectedFiles, QString& selectedFilter) {
    QFileDialog fileDlg(m_parentWidget, tr("Open Rastr files"));
    fileDlg.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDlg.setViewMode(QFileDialog::Detail);
    
    fileDlg.setFileMode(QFileDialog::ExistingFiles);
    
    QString qstr_filter = buildFileFilter();
    fileDlg.setNameFilter(qstr_filter);
    
    if (!m_currentDir.isEmpty()) {
        fileDlg.setDirectory(m_currentDir);
    }
    
    int n_res = fileDlg.exec();
    if (QDialog::Accepted != n_res) {
        return false;
    }
    
    selectedFiles = fileDlg.selectedFiles();
    selectedFilter = fileDlg.selectedNameFilter();
    
    return !selectedFiles.isEmpty();
}

QString FileManager::showSaveDialog() {
    QFileDialog fileDlg(m_parentWidget, tr("Save Rastr file"));
    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setOption(QFileDialog::DontUseNativeDialog, true);
    fileDlg.setViewMode(QFileDialog::Detail);
    
    QString qstr_filter = buildFileFilter();
    fileDlg.setNameFilter(qstr_filter);
    fileDlg.selectNameFilter("режим (*.rg2)");
    
    fileDlg.setFileMode(QFileDialog::AnyFile);
    
    if (!m_currentDir.isEmpty()) {
        fileDlg.setDirectory(m_currentDir);
    }
    
    int n_res = fileDlg.exec();
    if (QDialog::Accepted != n_res) {
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
    FormFileNew* pformFileNew = new FormFileNew(m_parentWidget);
    
    if (QDialog::Accepted != pformFileNew->exec()) {
        return false;
    }
    
    const FormFileNew::_s_checked_templatenames s_checked_templatenames = 
        pformFileNew->getCheckedTemplateNames();
    
    for (const FormFileNew::_s_checked_templatenames::value_type& templatename : s_checked_templatenames) {
        selectedTemplates.append(QString::fromStdString(templatename));
    }
    
    return !selectedTemplates.isEmpty();
}
