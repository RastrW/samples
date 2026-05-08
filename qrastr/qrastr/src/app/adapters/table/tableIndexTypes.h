#pragma once

/**
 * Сильные типы для пространств индексов таблицы Rastr.
 * Компилятор не даст случайно передать PluginIndex туда, где ждут RDataPos.
 *
 * ── Карта индексов ────────────────────────────────────────────────────
 *
 *  PluginIndex  = RCol::m_index
 *                 Номер колонки в плагине Rastr. Стабилен для данной
 *                 колонки независимо от загруженного файла.
 *                 Ключ в BackInfoCache (ENUM / NAMEREF / ENPIC / …).
 *                 НЕ совпадает с RDataPos в общем случае.
 *
 *  RDataPos     = позиция в vector<RCol>  =  mCols_[colName]
 *                 Номер колонки в RData (0..size-1).
 *                 Определяется порядком колонок в схеме конкретного файла(шаблона).
 *                 После setModel() совпадает с listIndex в QTitan.
 *                 Используется как «model column» в QAbstractTableModel::index().
 *
 *  LocalIndex   = позиция среди ЗАГРУЖЕННЫХ колонок внутри QDataBlock.
 *                 Хранится в RData::m_blockColIdx[rdataPos].
 *                 Блок содержит только запрошенные (видимые) колонки,
 *                 поэтому LocalIndex != RDataPos в общем случае.
 *                 Используется только внутри RData::getCell() — снаружи не нужен.
 *
 *  VisualIndex  = GridColumnBase::visualIndex()
 *                 Порядок отображения на экране (меняет пользователь).
 *                 Не связан ни с RDataPos, ни с PluginIndex.
 * ─────────────────────────────────────────────────────────────────────
 */

template<typename Tag, typename T>
struct StrongIndex {
    T value = -1;
    constexpr bool valid()   const noexcept { return value >= 0; }
    constexpr bool invalid() const noexcept { return value <  0; }

    constexpr explicit StrongIndex(T v) noexcept : value(v) {}
    constexpr StrongIndex() noexcept = default;

    constexpr bool operator==(StrongIndex o) const noexcept { return value == o.value; }
    constexpr bool operator!=(StrongIndex o) const noexcept { return value != o.value; }
    constexpr bool operator< (StrongIndex o) const noexcept { return value <  o.value; }
};

struct PluginIndexTag{};
struct RDataPosTag{};
struct LocalIndexTag{};
struct VisualIndexTag{};

using PluginIndex = StrongIndex<PluginIndexTag,  long>;
using RDataPos    = StrongIndex<RDataPosTag,      int>;
using LocalIndex  = StrongIndex<LocalIndexTag,    int>;
using VisualIndex = StrongIndex<VisualIndexTag,   int>;

// Sentinel: "все колонки" — используется в sig_dataChanged
inline constexpr RDataPos kAllColumns{ -2 };