#pragma once
#include <unordered_map>
#include <vector>
#include "condFormat.h"
#include "table/tableIndexTypes.h"
#include "table/tableIndexHash.h"
///@class Хранит unordered_map<col → [CondFormat]> для форматов по значению ячейки.
/// Управляет добавлением и заменой правил.
class CondFormatStorage
{
public:

    // Добавить или обновить одно правило для колонки column.
    // Условные правила идут в начало вектора (приоритет выше),
    // безусловные (пустой фильтр) — в конец.
    void add(ModelColumn column, const CondFormat& condFormat);
    // Полностью заменить список правил для колонки column.
    void set(ModelColumn column, const std::vector<CondFormat>& condFormats);

    // Прямой доступ для чтения
    const std::unordered_map<ModelColumn, std::vector<CondFormat>>& formats() const;
    // Удобный доступ к правилам одной колонки (возвращает nullptr, если колонки нет).
    const std::vector<CondFormat>* column(ModelColumn col) const;
private:
    static void addToMap(std::unordered_map<ModelColumn, std::vector<CondFormat>>& map,
                         ModelColumn column, const CondFormat& condFormat);

    std::unordered_map<ModelColumn, std::vector<CondFormat>> m_condFormats;
};
