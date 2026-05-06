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
    /// Перестроить карту rdata_pos → local_index.
    /// Вызывать после любого изменения состава колонок блока.
    void rebuildBlockIndexMap();
    /// Единая точка чтения ячейки: скрывает перевод индексов.
    /// Возвращает std::monostate если колонка не загружена.
    FieldVariantData getCell(int rdataPos, int row) const;
    /// Local index в блоке для позиции rdataPos, или -1 если не загружена.
    int blockColIndex(int rdataPos) const noexcept;
    /// Ленивая загрузка одной колонки в блок. Вызывается из const-контекста.
    /// Возвращает новый local index или -1 при ошибке.
    int ensureBlockCol(int rdataPos,
                       std::shared_ptr<ITableRepository> tables) const;
    /// Обновить индекс только для одной позиции (после lazy load).
    void updateBlockIndex(int rdataPos) const noexcept;
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

    std::string get_cols(bool visible = true) const;

    std::string t_name_;
    std::string t_title_;

    std::vector<std::string>          vCols_; ///< вектор имён колонок в порядке следования.
    std::unordered_map<std::string, int> mCols_; ///< unordered_map<имя_колонки, индекс> для быстрого поиска колонки по имени.
private:
    std::shared_ptr<QDataBlock>       datablock;
    mutable std::vector<int> m_blockColIdx; ///< rdata_pos → local block index
};
