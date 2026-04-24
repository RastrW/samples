#pragma once

#include "rcol.h"
#include "table/rtablesdatamanager.h"

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
     * После вызова pnparray_ указывает на тот же объект,
     * что и у других открытых окон этой таблицы.
     */
    void populateBlock(ITableRepository* repo);

    int         AddCol(const RCol& rcol);
    std::string get_cols(bool visible = true) const;
    std::string getCommaSeparatedFieldNames() const;

    std::string t_name_;
    std::string t_title_;

    std::vector<std::string>          vCols_; ///< вектор имён колонок в порядке следования.
    std::shared_ptr<QDataBlock>       pnparray_;
    std::unordered_map<std::string, int> mCols_; ///< unordered_map<имя_колонки, индекс> для быстрого поиска колонки по имени.
private:
    std::string m_str_cols; ///< vCols_ в виде строки имен столбцов ex: "ny,pn,qn,vras"
};
