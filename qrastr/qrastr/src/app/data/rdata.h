#pragma once

#include "astra_headers/UIForms.h"
#include "rtablesdatamanager.h"
using WrapperExceptionType = std::runtime_error;
#include "QDataBlocks.h"

#include <astra/IPlainRastrWrappers.h>
#include "rcol.h"

typedef std::vector<std::string> _vstr;

/**
 * @brief Метаданные таблицы: вектор RCol + ссылка на общий плоский массив данных.
 * pnparray_ — shared_ptr на QDataBlock в RTablesDataManager.
 * Несколько открытых окон одной таблицы разделяют ОДИН блок памяти.
 * Поэтому RTablesDataManager::get() возвращает shared_ptr, а не копию.
 */
class RData
    : public std::vector<RCol> {
public:
    /** Строит вектор RCol по метаданным плагина.
     * @note reserve() вызывается ДО push_back, иначе при reallocate
     * указатели на элементы вектора инвалидируются и RCol теряют данные.
     */
    RData(QAstra* _pqastra, const CUIForm& _form);
    QAstra* getAstra() const { return m_qastra; }

    int AddCol(const RCol& rcol){
        emplace_back(rcol);
        return static_cast<int>(size());
    }
    /**
     * Получает (или создаёт, если ещё нет) общий QDataBlock из RTablesDataManager.
     * После этого вызова pnparray_ указывает на тот же объект, что и у других
     * открытых окон этой таблицы — данные НЕ дублируются.
     */
    void populate_qastra(QAstra* _pqastra,RTablesDataManager* _pRTDM);
    std::string get_cols(bool visible = true);

    std::string getCommaSeparatedFieldNames();

    std::string t_name_ = "";
    std::string t_title_ = "";

    std::vector<std::string> vCols_;   ///< вектор имён колонок в порядке следования.
    std::shared_ptr<QDataBlock> pnparray_;
    std::map<std::string, int> mCols_; ///< map<имя_колонки, индекс> для быстрого поиска колонки по имени.
private:
    std::string m_str_cols = "";       ///< vCols_ в виде строки имен столбцов ex: "ny,pn,qn,vras"
    QAstra* m_qastra;
};
