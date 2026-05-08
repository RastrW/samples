#pragma once
#include <unordered_map>
#include <vector>
#include "condFormat.h"
#include "table/tableIndexTypes.h"

///@class Хранит unordered_map<col → [CondFormat]> для форматов по значению ячейки.
/// Управляет добавлением и заменой правил.
class CondFormatStorage
{
public:

    // Добавить или обновить одно правило для колонки column.
    // Условные правила идут в начало вектора (приоритет выше),
    // безусловные (пустой фильтр) — в конец.
    void add(RDataPos column, const CondFormat& condFormat);
    // Полностью заменить список правил для колонки column.
    void set(RDataPos column, const std::vector<CondFormat>& condFormats);

    // Прямой доступ для чтения
    const std::unordered_map<RDataPos, std::vector<CondFormat>>& formats() const;
    // Удобный доступ к правилам одной колонки (возвращает nullptr, если колонки нет).
    const std::vector<CondFormat>* column(RDataPos col) const;
private:
    static void addToMap(std::unordered_map<RDataPos, std::vector<CondFormat>>& map,
                         RDataPos column, const CondFormat& condFormat);

    std::unordered_map<RDataPos, std::vector<CondFormat>> m_condFormats;
};
