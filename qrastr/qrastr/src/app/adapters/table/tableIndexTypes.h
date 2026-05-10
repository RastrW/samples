#pragma once

/**
 * Сильные типы для пространств индексов таблицы Rastr.
 *
 * ── ЗАЧЕМ это нужно ──────────────────────────────────────────────────
 *  Без сильных типов компилятор не отличает:
 *      getCell(modelColumn, row)   — правильно
 *      getCell(localIndex, row) — молча сломано (читает не ту колонку)
 *      getCell(astraIndex, row)— молча сломано (индекс из другого пространства)
 *  Ошибки проявляются только в рантайме, при редких сочетаниях видимых колонок.
 *  StrongIndex делает такие ошибки ошибками компиляции.
 *
 * ── КАРТА ИНДЕКСОВ ───────────────────────────────────────────────────
 *
 *  AstraIndex  = RCol::m_index
 *                 Номер колонки в COM-плагине Rastr.
 *                 Стабилен для данного типа таблицы, не зависит от файла.
 *                 Ключ в BackInfoCache (ENUM / NAMEREF / ENPIC / …).
 *                 Источник: schema.index при getSchema().
 *                 НЕ совпадает с ModelColumn в общем случае.
 *
 *  ModelColumn  = позиция в vector<RCol>  =  mCols_[colName]
 *                 Номер колонки в QAbstractTableModel: QModelIndex::column(),
 *                 binding->column(), addColumn(n).
 *                 Определяется порядком колонок в схеме конкретного файла(шаблона).
 *                 Диапазон: [0, RData::size()).
 *
 *  LocalIndex   = позиция среди ЗАГРУЖЕННЫХ колонок внутри QDataBlock.
 *                 Хранится в RData::m_blockColIdx[modelColumn].
 *                 Блок содержит только запрошенные (видимые) колонки,
 *                 поэтому LocalIndex != ModelColumn в общем случае.
 *                 ИСПОЛЬЗУЕТСЯ ТОЛЬКО внутри RData::getCell() — наружу не выходит.
 *
 *  VisualIndex  = GridColumnBase::visualIndex()
 *                 Порядок отображения на экране (меняет пользователь drag-n-drop).
 *                 Не связан ни с ModelColumn, ни с AstraIndex.
 *                 Хранится только в RtabController для восстановления после сброса.
 *
 * ── ПРАВИЛА РАБОТЫ НА ГРАНИЦАХ ───────────────────────────────────────
 *
 *  QAbstractTableModel → ModelColumn:
 *      ModelColumn col{ index.column() };   // из QModelIndex
 *
 *  QTitan binding → ModelColumn:
 *      ModelColumn col{ binding->column() }; // binding->column() == model column
 *
 *  ModelColumn → Qt/QTitan (требует int):
 *      emit dataChanged(index(row, col.value), ...);  // .value — только на стыке с Qt
 *      m_view->getColumnByModelColumn(col.value);
 *
 *  ModelColumn → контейнер (требует size_t):
 *      (*this)[col.to_size()]          // to_size() — только для operator[]
 *      m_blockColIdx[col.to_size()]
 *
 *  ModelColumn → AstraIndex (через метаданные, НЕ напрямую):
 *      AstraIndex pi = rdata.colAt(col)->astraIndex();
 *
 *  LocalIndex НИКОГДА не покидает RData:
 *      // Снаружи: rdata.getCell(col, row) — не getCell(localIdx, row)
 *
 *  VisualIndex НИКОГДА не используется как ключ в данных:
 *      // Только col->setVisualIndex(vi.value) и col->visualIndex()
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

struct AstraIndexTag{};
struct ModelColumnTag{};
struct LocalIndexTag{};
struct VisualIndexTag{};

using AstraIndex = StrongIndex<AstraIndexTag, long>;
using ModelColumn    = StrongIndex<ModelColumnTag, int>;
using LocalIndex  = StrongIndex<LocalIndexTag, long>;
using VisualIndex = StrongIndex<VisualIndexTag, int>;
