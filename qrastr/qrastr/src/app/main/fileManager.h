#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QMap>
#include <memory>

class QAstra;

/**
 * @brief Класс для управления файловыми операциями
 * 
 * Отвечает за:
 * - Открытие и сохранение файлов через QAstra
 * - Управление списком недавних файлов
 * - Валидацию и проверку шаблонов
 * - Диалоги выбора файлов
 */
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
    
    /// @brief Открыть файлы через диалог
    int openFiles();
    
    ///@brief Открыть конкретный файл
    bool openFile(const QString& filePath, const QString& templatePath = "");
    
    ///@brief Сохранить текущий файл
    bool save();
    
    /// @brief Сохранить файл с новым именем
    bool saveAs();
    
    ///@brief Сохранить ВСЕ загруженные файлы через форму formsaveall
    bool saveAll();
    
    // ========== Управление загруженными файлами ==========

    QString currentFile() const { return m_currentFile; }
    QString currentDirectory() const { return m_currentDir; }

    /**
     * @brief Получить карту ВСЕХ загруженных файлов
     * @return map<template, file>
     */
    const QMap<QString, QString>& loadedFiles() const { return m_loadedFiles; }

    /**
     * @brief Установить текущий файл И добавить в карту загруженных
     */
    void setCurrentFile(const QString& fileName, const QString& templatePath = "");

    // ========== Недавние файлы ==========
    ///@brief Добавить файл в список недавних
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
    ///@brief Файл успешно открыт
    void fileOpened(const QString& filePath);
    void filesOpened(int count);
    ///@brief Файл успешно сохранён
    void fileSaved(const QString& filePath);
    ///@brief Ошибка при загрузке файла
    void fileLoadError(const QString& error);
    ///@brief Список недавних файлов изменился
    void recentFilesChanged();
    /// @brief Изменился текущий файл
    void currentFileChanged(const QString& filePath);
private:
    // Зависимости
    std::shared_ptr<QAstra> m_qastra;
    QWidget* m_parentWidget;
    // Состояние
    QString m_currentFile;
    QString m_currentDir;
    QMap<QString, QString> m_loadedFiles;  // template -> file
    // Константы
    static constexpr int MaxRecentFiles = 10;
    static constexpr const char* RecentFilesKey = "recentFileList";
    /// @brief Показать диалог открытия файла
    bool showOpenDialog(QStringList& selectedFiles, QString& selectedFilter);
    ///@brief Показать диалог сохранения файла
    QString showSaveDialog();
    /// @brief Построить фильтр для диалога по расширениям
    QString buildFileFilter() const;
    /// @brief Определить шаблон по расширению файла
    QString findTemplateByExtension(const QString& filePath) const;
    /// @brief Показать форму выбора шаблонов для нового файла
    bool showNewFileDialog(QStringList& selectedTemplates);
};
