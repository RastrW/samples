#pragma once

#include "rcol.h"
#include "table/rTablesDataAdapter.h"

class CUIForm;
class QDataBlock;

/**
 * @brief Метаданные таблицы: вектор RCol + ссылка на общий блок данных.
 * Несколько открытых окон одной таблицы разделяют ОДИН блок памяти.
 */
class RData
    : public std::vector<RCol> {
public:
    /**
     * @param schema  Схема таблицы, полученная из ITableRepository::getSchema().
     *                Передаётся по const& — RData не владеет схемой,
     *                копирует из неё только нужные строки.
     * @param form    Описание формы UI (видимые колонки, порядок).
     */
    RData(const ITableRepository::TableSchema& schema,
          const CUIForm&                        form);

    /**
     * Получает общий QDataBlock из репозитория.
     * После вызова datablock указывает на тот же объект,
     * что и у других открытых окон этой таблицы.
     * Загружаем только колонки формы
     */
    void populateBlock(std::shared_ptr<ITableRepository> tables);
    /// Local index в блоке для позиции rdataPos, или -1 если не загружена.
    int blockColIndex(int rdataPos) const noexcept;
    int getRowsCount() const{
        return static_cast<int>(datablock->RowsCount());
    }
    int getColumnsCount() const{
        return static_cast<int>(datablock->ColumnsCount());
    }
    void duplicateRow(int row){
        datablock->DuplicateRow(row);
    }
    bool isReady() const{
        return datablock != nullptr;
    }
    // ── Конвертация между пространствами индексов ────────────────────────
    /// colName → RDataPos. O(1) через mCols_. Возвращает invalid() если нет.
    RDataPos rdataPosOf(const std::string& colName) const noexcept;
    /// RDataPos → PluginIndex (через метаданные RCol).
    PluginIndex pluginIndexOf(RDataPos pos) const noexcept;
    /// RDataPos → LocalIndex в QDataBlock. -1 если колонка не загружена.
    LocalIndex localIndexOf(RDataPos pos) const noexcept;
    /// RCol по RDataPos — безопасный доступ.
    const RCol* colAt(RDataPos pos) const noexcept;
    // ── Загрузка и чтение ─────────────────────────────────────────────────
    /// Ленивая загрузка колонки в блок. Возвращает LocalIndex или invalid().
    LocalIndex ensureLoaded(RDataPos pos,
                            std::shared_ptr<ITableRepository> tables) const;
    /// Единственная точка чтения ячейки.
    /// Принимает RDataPos — снаружи local_index не нужен.
    FieldVariantData getCell(RDataPos pos, int row) const;

    std::string get_cols(bool visible = true) const;
    std::vector<std::string> colNames() const;

    std::string t_name_;
    std::string t_title_;
    std::unordered_map<std::string, int> mCols_; ///< unordered_map<имя_колонки, индекс> для быстрого поиска колонки по имени.
private:
    /// Обновить индекс только для одной позиции (после lazy load).
    void updateBlockIndex(RDataPos pos) const noexcept;
    /// Перестроить карту rdata_pos → local_index.
    /// Вызывать после любого изменения состава колонок блока.
    void rebuildBlockIndexMap();
    std::shared_ptr<QDataBlock>       datablock;
    mutable std::vector<int> m_blockColIdx; /// rdata_pos → local_index
};
