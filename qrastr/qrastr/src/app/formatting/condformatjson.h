#pragma once
#include "json.hpp"
#include <QDir>

class CondFormat;

///@class Сохранение и загрузка условий форматирования в JSON.
/// Форматы хранятся и возвращаются по имени колонки — без привязки к позиции.
class CondFormatJson
{
public:
    /// Загрузить форматы из файла для таблицы tableName.
    /// Возвращает карту colName → [CondFormat].
    /// Пустую, если файл не найден или секция таблицы отсутствует.
    static std::unordered_map<std::string, std::vector<CondFormat>>
    load(const std::string& tableName);

    /// Сохранить форматы для таблицы tableName в общий JSON-файл.
    /// Если файл уже существует — обновляет только секцию этой таблицы,
    /// остальные таблицы в файле не трогает.
    static void save(
        const std::string& tableName,
        const std::unordered_map<std::string, std::vector<CondFormat>>& formats);

private:
    static QString jsonPath() {
        return QDir::current().filePath("highlightsettings.json");
    }

    /// Сериализовать форматы одной таблицы в JSON-объект.
    /// Ключ JSON-объекта — имя колонки.
    static nlohmann::json tableToJson(
        const std::unordered_map<std::string, std::vector<CondFormat>>& formats);

    static constexpr const char kRoot[]    = "condformat";
    static constexpr const char kFilter[]  = "filter";
    static constexpr const char kBgColor[] = "bgColor";
    static constexpr const char kFgColor[] = "fgColor";
};
