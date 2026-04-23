#pragma once
#include "QDataBlocks.h"
#include "astra_shared.h"

class CUIForm;

class ITableRepository
{
public:
    virtual ~ITableRepository() = default;

    // ── Схема ────────────────────────────────────────────────────────────────
    struct ColumnSchema {
        std::string name;
        std::string tableName;
        std::string title;
        std::string description;
        std::string unit;
        std::string width;
        std::string prec;
        std::string expression;
        std::string nameRef;
        std::string afor;
        std::string ff;
        std::string min;
        std::string max;
        std::string scale;
        std::string cache;
        enComPropTT comPropTT = enComPropTT::COM_PR_INT;
        long        index     = -1;
    };

    struct TableSchema {
        std::string              name;
        std::string              title;
        std::vector<ColumnSchema> columns; // в порядке индексов плагина
    };

    virtual TableSchema getSchema(const std::string& tname) = 0;

    // ── Данные ───────────────────────────────────────────────────────────────
    virtual std::shared_ptr<QDataBlock>
        getBlock(const std::string& tname,
                 const std::string& cols) = 0;

    virtual void fillBlock(const std::string& tname,
                           QDataBlock&        block,
                           const std::string& cols = "",
                           std::optional<FieldDataOptions> opts = std::nullopt) = 0;

    virtual long columnIndex(const std::string& tname,
                             const std::string& colName) = 0;

    virtual std::vector<long>
        rowsBySelection(const std::string& tname,
                        const std::string& selection) = 0;

    // ── Запись ───────────────────────────────────────────────────────────────
    virtual void setValue(const std::string&      tname,
                          const std::string&      colName,
                          long                    row,
                          const FieldVariantData& value) = 0;

    virtual void setColumnProperty(const std::string& tname,
                                   const std::string& colName,
                                   FieldProperties    prop,
                                   const std::string& value) = 0;

    virtual void calcColumn(const std::string& tname,
                            const std::string& colName,
                            const std::string& expression,
                            const std::string& selection) = 0;

    virtual void insertRows(const std::string& tname,
                            long               startRow,
                            int                count) = 0;

    virtual void deleteRows(const std::string& tname,
                            long               startRow,
                            int                count) = 0;

    virtual void addRows   (const std::string& tname,
                         size_t             count) = 0;
    virtual void duplicateRow(const std::string& tname, long row) = 0;

    virtual long tableSize(const std::string& tname) = 0;
    virtual void setTableSize(const std::string& tname, long size) = 0;
    //──Администрирование ───────────────────────────────────────────────
    virtual void exportToCsv(const std::string& tname,
                             const std::string& cols,
                             const std::string& selection,
                             const std::string& path,
                             const std::string& divider,
                             eCSVCode           mode) = 0;
    virtual void importToCsv(const std::string& tname,
                             const std::string& cols,
                             const std::string& selection,
                             const std::string& file,
                             const std::string& divider,
                             const std::string& byDefault,
                             eCSVCode           mode) = 0;
    virtual void setLockEvent(bool lock) = 0;

    virtual CUIForm* getForm(std::string name)= 0;
};
