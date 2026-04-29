#pragma once
#include <memory>
#include <QObject>
#include "recentFilesManager.h"

enum class eLoadCode;
class IFileOperations;


/// @class Менеджер файловых операций
class FileManager : public QObject {
    Q_OBJECT
    
public:
    explicit FileManager(
        std::shared_ptr<IFileOperations> fileOps,
        QWidget* parent = nullptr
    );
    ~FileManager() = default;
    
    // ========== Основные операции ==========
    /// @brief Создать новый файл с выбором шаблонов
    bool newFile(); 
    /**
     * @brief Открыть файлы через диалог
     * @return количество успешно открытых файлов
     * @note Поддерживает выбор нескольких файлов
     */
    int openFiles(); 
    /**
     * @brief Открыть конкретный файл
     * @param filePath путь к файлу
     * @param templatePath путь к шаблону (может быть пустым)
     */
    bool openFile(const QString& filePath,
                  const QString& templatePath = "");
    /// @brief Сохранить текущий файл
    bool save();
    /// @brief Сохранить файл с новым именем
    bool saveAs();
    /**
     * @brief Сохранить все загруженные файлы
     * @note Передаёт m_loadedFiles в formsaveall
     */
    bool saveAll();
    
    // ========== Управление загруженными файлами ==========
    /// @brief Получить текущий активный файл
    QString currentFile() const { return m_currentFile; }
    /// @brief Получить текущую директорию
    QString currentDirectory() const { return m_currentDir; }
    /**
     * @brief Получить карту ВСЕХ загруженных файлов
     * @return map<template, file>
     */
    const std::unordered_map<QString, QString>& loadedFiles() const { return m_loadedFiles; }
    /// @brief Установить текущий файл И добавить в карту загруженных
    void setCurrentFile(const QString& fileName, const QString& templatePath = "");
    
    // ========== Недавние файлы ==========
    /// @brief Получить список недавних файлов
    [[nodiscard]]
    QList<RecentFileEntry> getRecentFiles() const;
    /// @brief Открыть файл из списка недавних
    void openRecentFile(const QString& filePath, const QString& templatePath);
signals:
    /// @brief Файл успешно открыт
    void sig_fileOpened(const QString& filePath);
    
    /**
     * @brief Несколько файлов открыто
     * @param count количество файлов
     */
    void sig_filesOpened(int count);
    /// @brief Файл успешно сохранён
    void sig_fileSaved(const QString& filePath);
    /// @brief Ошибка при загрузке файла
    void sig_fileLoadError(const QString& error);
    /// @brief Текущий файл изменился
    void sig_currentFileChanged(const QString& filePath);
private:
    static constexpr const char* k_noTemplateFilter = "No template (*)";
    // ========== Зависимости ==========
    std::shared_ptr<IFileOperations> m_fileOps;
    QWidget* m_parentWidget;
    
    // ========== Состояние файлов ==========
    QString m_currentFile;
    QString m_currentDir;
    std::unordered_map<QString, QString> m_loadedFiles; // key=templatePath, value=filePath
    RecentFilesManager               m_recentFiles;
    // ========== Вспомогательные методы ==========
    /**
     * @brief Показать диалог открытия файлов
     * @param selectedFiles [out] список выбранных файлов
     * @param selectedFilter [out] выбранный фильтр
     * @return true если пользователь выбрал файлы
     */
    bool showOpenDialog(QStringList& selectedFiles,
                        QString& selectedFilter);
    /// @brief Показать диалог сохранения файла
    QString showSaveDialog();
    /**
     * @brief Построить фильтр для диалога
     * @return строка фильтра вида "Known types(...);; No template (*);;..."
     */
    QString buildFileFilter() const;
    /// @brief Найти шаблон по расширению файла
    QString findTemplateByExtension(const QString& filePath) const;
    /// @brief Показать диалог выбора шаблонов для нового файла
    bool showNewFileDialog(QStringList& selectedTemplates);
    void registerStartupFile(const QString& fileName,
                             const QString& templatePath);
};
