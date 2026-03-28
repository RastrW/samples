#pragma once
#include <unordered_map>
#include <vector>

class CondFormat;

///@class Хранит unordered_map<col → [CondFormat]> для форматов по значению ячейки.
/// Управляет добавлением и заменой правил.
class CondFormatStorage
{
public:
    // Добавить или обновить одно правило для колонки column.
    // Условные правила идут в начало вектора (приоритет выше),
    // безусловные (пустой фильтр) — в конец.
    void add(std::size_t column, const CondFormat& condFormat);
    // Полностью заменить список правил для колонки column.
    void set(std::size_t column, const std::vector<CondFormat>& condFormats);

    // Прямой доступ для чтения
    const std::unordered_map<std::size_t, std::vector<CondFormat>>& formats() const;
    // Удобный доступ к правилам одной колонки (возвращает nullptr, если колонки нет).
    const std::vector<CondFormat>* column(std::size_t col) const;
private:
    static void addToMap(std::unordered_map<std::size_t, std::vector<CondFormat>>& map,
                         std::size_t column, const CondFormat& condFormat);

    std::unordered_map<std::size_t, std::vector<CondFormat>> m_condFormats;
};
