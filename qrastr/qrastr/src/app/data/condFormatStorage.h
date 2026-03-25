#pragma once
#include <unordered_map>
#include <vector>

class CondFormat;

///@class  Хранит два типа условных форматов и управляет их добавлением / заменой.
class CondFormatStorage
{
public:
    // isRowIdFormat == true  → форматы по идентификатору строки (приоритет выше)
    // isRowIdFormat == false → форматы по значению ячейки
    void add(bool isRowIdFormat, std::size_t column, const CondFormat& condFormat);
    void set(bool isRowIdFormat, std::size_t column, const std::vector<CondFormat>& condFormats);

    const std::unordered_map<std::size_t, std::vector<CondFormat>>& rowIdFormats() const {
        return m_rowIdFormats;
    }
    const std::unordered_map<std::size_t, std::vector<CondFormat>>& condFormats()  const {
        return m_condFormats;
    }
private:
    static void addToMap(std::unordered_map<std::size_t, std::vector<CondFormat>>& map,
                         std::size_t column, const CondFormat& condFormat);

    std::unordered_map<std::size_t, std::vector<CondFormat>> m_rowIdFormats;
    std::unordered_map<std::size_t, std::vector<CondFormat>> m_condFormats;
};
