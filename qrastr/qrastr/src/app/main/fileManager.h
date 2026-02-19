#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QMap>
#include <memory>

class QAstra;
enum class eLoadCode;
enum class IPlainRastrRetCode;

/// @class Менеджер файловых операций
class FileManager : public QObject {
    Q_OBJECT
    
public:
    explicit FileManager(
        std::shared_ptr<QAstra> qastra,
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
    const QMap<QString, QString>& loadedFiles() const { return m_loadedFiles; }
    /// @brief Установить текущий файл И добавить в карту загруженных
    void setCurrentFile(const QString& fileName, const QString& templatePath = "");
    
    // ========== Недавние файлы ==========
    /// @brief Добавить файл в список недавних
    void addToRecentFiles(const QString& filePath, const QString& templatePath = "");
    /**
     * @brief Получить список недавних файлов
     * @return список строк вида "file <template>"
     */
    QStringList getRecentFiles() const;
    
    /**
     * @brief Открыть файл из списка недавних
     * @param fileAndTemplate строка вида "file <template>"
     */
    void openRecentFile(const QString& fileAndTemplate);
    
signals:
    /// @brief Файл успешно открыт
    void fileOpened(const QString& filePath);
    
    /**
     * @brief Несколько файлов открыто
     * @param count количество файлов
     */
    void filesOpened(int count);
    /// @brief Файл успешно сохранён
    void fileSaved(const QString& filePath);
    /// @brief Ошибка при загрузке файла
    void fileLoadError(const QString& error);
    /// @brief Список недавних файлов изменился
    void recentFilesChanged();
    /// @brief Текущий файл изменился
    void currentFileChanged(const QString& filePath);   
private:
    // ========== Зависимости ==========
    std::shared_ptr<QAstra> m_qastra;
    QWidget* m_parentWidget;
    
    // ========== Состояние файлов ==========
    QString m_currentFile;
    QString m_currentDir;
    QMap<QString, QString> m_loadedFiles;
    // ========== Константы ==========
    static constexpr int m_maxRecentFiles = 10;
    static constexpr const char* m_recentFilesKey = "recentFileList";
    
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
};
