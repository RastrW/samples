#include "condFormatStorage.h"
#include "condFormat.h"

void CondFormatStorage::addToMap(std::unordered_map<size_t, std::vector<CondFormat>>& map,
                                 size_t column, const CondFormat& condFormat)
{
    // If the condition is already present in the vector, update that entry and respect the order, since two entries with the same
    // condition do not make sense.
    auto& vec = map[column];
    auto it = std::find_if(vec.begin(), vec.end(), [&](const CondFormat& f) {
        return f.sqlCondition() == condFormat.sqlCondition();
    });
    if (it != vec.end()) {
        *it = condFormat;                        // обновить существующий
    } else if (condFormat.filter().isEmpty()) {
        vec.push_back(condFormat);               // безусловный — в конец
    } else {
        vec.insert(vec.begin(), condFormat);     // условный — в начало (приоритет)
    }
}

void CondFormatStorage::add(bool isRowIdFormat, size_t column, const CondFormat& condFormat)
{
    addToMap(isRowIdFormat ? m_rowIdFormats : m_condFormats, column, condFormat);
}

void CondFormatStorage::set(bool isRowIdFormat, size_t column,
                            const std::vector<CondFormat>& condFormats)
{
    auto& map = isRowIdFormat ? m_rowIdFormats : m_condFormats;
    map[column] = condFormats;
}
