#pragma once
#include <map>
#include <vector>

class CondFormat;

///@class  Хранит два типа условных форматов и управляет их добавлением / заменой.
class CondFormatStorage
{
public:
    // isRowIdFormat == true  → форматы по идентификатору строки (приоритет выше)
    // isRowIdFormat == false → форматы по значению ячейки
    void add(bool isRowIdFormat, size_t column, const CondFormat& condFormat);
    void set(bool isRowIdFormat, size_t column, const std::vector<CondFormat>& condFormats);

    const std::map<size_t, std::vector<CondFormat>>& rowIdFormats() const { return m_rowIdFormats; }
    const std::map<size_t, std::vector<CondFormat>>& condFormats()  const { return m_condFormats;  }

private:
    static void addToMap(std::map<size_t, std::vector<CondFormat>>& map,
                         size_t column, const CondFormat& condFormat);

    std::map<size_t, std::vector<CondFormat>> m_rowIdFormats;
    std::map<size_t, std::vector<CondFormat>> m_condFormats;
};
