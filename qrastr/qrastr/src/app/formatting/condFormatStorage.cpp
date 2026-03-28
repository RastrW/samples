#include "condFormatStorage.h"
#include "condFormat.h"

void CondFormatStorage::addToMap(
    std::unordered_map<size_t, std::vector<CondFormat>>& map,
    size_t column,
    const CondFormat& condFormat)
{
    auto& vec = map[column];
    auto it = std::find_if(vec.begin(), vec.end(), [&](const CondFormat& f) {
        return f.sqlCondition() == condFormat.sqlCondition();
    });

    if (it != vec.end()) {
        *it = condFormat;                   // обновить существующее правило
    } else if (condFormat.filter().isEmpty()) {
        vec.push_back(condFormat);          // безусловное — в конец
    } else {
        vec.insert(vec.begin(), condFormat); // условное — в начало (приоритет)
    }
}

void CondFormatStorage::add(size_t column, const CondFormat& condFormat){
    addToMap(m_condFormats, column, condFormat);
}

void CondFormatStorage::set(size_t column, const std::vector<CondFormat>& condFormats){
    m_condFormats[column] = condFormats;
}

const std::unordered_map<std::size_t, std::vector<CondFormat>>&
CondFormatStorage::formats() const
{
    return m_condFormats;
}
// Удобный доступ к правилам одной колонки (возвращает nullptr, если колонки нет).
const std::vector<CondFormat>* CondFormatStorage::column(std::size_t col) const
{
    auto it = m_condFormats.find(col);
    return (it != m_condFormats.end()) ? &it->second : nullptr;
}