#pragma once
#include <QObject>

class IFileOperations;
class QDir;

/**
 * @brief Загружает стартовые шаблоны и файлы.
 * Ответственности этого класса:
 *  - Загрузить шаблоны из Params::getStartLoadTemplates().
 *  - Загрузить файлы из Params::getStartLoadFileTemplates().
 *  - Показать пользователю сообщение при ошибке (через QMessageBox).
 *  - Шаблон — критичная ошибка (load() вернёт false).
 *    Отсутствующий файл — некритично.
 */
class StartupLoader : public QObject {
    Q_OBJECT

public:
    explicit StartupLoader(std::shared_ptr<IFileOperations> fileOps,
                           QWidget*                parentWidget = nullptr,
                           QObject*                parent       = nullptr);

    /**
     * @brief Выполнить загрузку.
     * @return false только при критической ошибке (не удалось загрузить шаблон).
     *         Пропущенные файлы — не критичны, возвращаем true.
     */
    bool load();

signals:
    /// Некритическая проблема при загрузке файла (файл не найден и т.п.)
    void sig_loadWarning(const QString& message);

private:
    bool loadTemplates(const QDir& templatesDir);
    void loadFiles(const QDir& templatesDir);

    std::shared_ptr<IFileOperations> m_fileOps;
    QWidget*                m_parentWidget;
};
