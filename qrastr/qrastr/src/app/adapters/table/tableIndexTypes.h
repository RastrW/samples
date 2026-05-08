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
 *  VisualIndex  = GridColumnBase::int visualIndex()
 *                 Порядок отображения на экране (меняет пользователь).
 *                 Не связан ни с RDataPos, ни с PluginIndex.
 * ─────────────────────────────────────────────────────────────────────
 */
template<typename Tag, typename T>
struct StrongIndex {
    T value = -1;

    // Только explicit конструктор от T
    explicit constexpr StrongIndex(T v) noexcept : value(v) {}
    // Конструктор по умолчанию
    constexpr StrongIndex() noexcept = default;
    // Запрещаем копирование из других типов StrongIndex
    template<typename OtherTag, typename OtherT>
    StrongIndex(const StrongIndex<OtherTag, OtherT>&) = delete;
    //Запрещаем присваивание из T
    StrongIndex& operator=(T) = delete;
    //Запрещаем неявное преобразование в T
    operator T() const = delete;
    //Разрешаем копирование только своего типа
    constexpr StrongIndex(const StrongIndex&) = default;
    constexpr StrongIndex& operator=(const StrongIndex&) = default;

    constexpr bool valid()   const noexcept { return value >= 0; }
    constexpr bool invalid() const noexcept { return value <  0; }
    // Проверка на валидность относительно размера контейнера
    constexpr bool valid_in(T size) const noexcept {
        return valid() && value < size;
    }
    // Проверка с автоматическим приведением size_t (без warnings)
    constexpr bool valid_in(size_t container_size) const noexcept {
        return valid() && static_cast<size_t>(value) < container_size;
    }
    // Безопасное преобразование в size_t для доступа к контейнеру
    size_t to_size() const noexcept {
        return static_cast<size_t>(value);
    }
    constexpr bool operator==(StrongIndex o) const noexcept { return value == o.value; }
    constexpr bool operator!=(StrongIndex o) const noexcept { return value != o.value; }
    constexpr bool operator< (StrongIndex o) const noexcept { return value <  o.value; }
    constexpr bool operator<=(StrongIndex o) const noexcept { return value <= o.value; }
    constexpr bool operator> (StrongIndex o) const noexcept { return value >  o.value; }
    constexpr bool operator>=(StrongIndex o) const noexcept { return value >= o.value; }

    constexpr StrongIndex& operator++() noexcept { ++value; return *this; }
    constexpr StrongIndex  operator++(int) noexcept { auto tmp = *this; ++value; return tmp; }
    constexpr StrongIndex& operator--() noexcept { --value; return *this; }
    constexpr StrongIndex  operator--(int) noexcept { auto tmp = *this; --value; return tmp; }

    constexpr StrongIndex operator+(T offset) const noexcept { return StrongIndex(value + offset); }
    constexpr StrongIndex operator-(T offset) const noexcept { return StrongIndex(value - offset); }

    constexpr StrongIndex& operator+=(T offset) noexcept { value += offset; return *this; }
    constexpr StrongIndex& operator-=(T offset) noexcept { value -= offset; return *this; }
};

struct PluginIndexTag{};
struct RDataPosTag{};
struct LocalIndexTag{};
struct VisualIndexTag{};

using PluginIndex = StrongIndex<PluginIndexTag, long>;
using RDataPos    = StrongIndex<RDataPosTag, int>;
using LocalIndex  = StrongIndex<LocalIndexTag, long>;
using VisualIndex = StrongIndex<VisualIndexTag, int>;
