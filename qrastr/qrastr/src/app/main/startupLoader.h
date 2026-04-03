#pragma once
#include <QObject>
#include "qastra.h"

/**
 * @brief Загружает стартовые шаблоны и файлы через m_qastra->Load().
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
    explicit StartupLoader(std::shared_ptr<QAstra> qastra,
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
    void loadWarning(const QString& message);

private:
    bool loadTemplates(const std::filesystem::path& templatesDir);
    bool loadFiles(const std::filesystem::path& templatesDir);

    std::shared_ptr<QAstra> m_qastra;
    QWidget*                m_parentWidget;
};