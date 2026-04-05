#pragma once
#include "json.hpp"
#include <filesystem>

class CondFormat;

///@class Сохранение и загрузка условий в json
class CondFormatJson
{
public:
    // Загрузить форматы из файла для таблицы tableName.
    // cols — упорядоченный список имён колонок (индекс == позиция в таблице).
    // Возвращает карту col_index → [CondFormat]; пустую, если файл не найден.
    static std::unordered_map<int, std::vector<CondFormat>>
    load(const std::string& tableName,
         const std::vector<std::string>& cols);

    // Сохранить форматы для таблицы tableName в общий JSON-файл.
    // Если файл уже существует — обновляет только секцию этой таблицы,
    // остальные таблицы в файле не трогает.
    static void save(const std::string& tableName,
                     const std::vector<std::string>& cols,
                     const std::unordered_map<int, std::vector<CondFormat>>& formats);

private:
    // Путь к файлу — константа, не нужно передавать через конструктор.
    static std::filesystem::path jsonPath()
    {
        return std::filesystem::current_path() / "highlightsettings.json";
    }

    // Сериализовать форматы одной таблицы в JSON-объект.
    static nlohmann::json tableToJson(
        const std::vector<std::string>& cols,
        const std::unordered_map<int, std::vector<CondFormat>>& formats);

    static constexpr const char kRoot[]    = "condformat";
    static constexpr const char kFilter[]  = "filter";
    static constexpr const char kBgColor[] = "bgColor";
    static constexpr const char kFgColor[] = "fgColor";
};
