#pragma once

#include "astra_shared.h"
#include "table/ITableRepository.h"

/// @brief Метаданные и вспомогательные операции для одной колонки таблицы Rastr.
class RCol
{
public:
    enum class _en_data {
        DATA_ERR  = -1,
        DATA_BOOL =  0,
        DATA_INT  =  1,
        DATA_DBL  =  2,
        DATA_STR  =  3
    };

    RCol() = default;
    ~RCol() = default;

    /// Заполняется один раз при построении RData.
    /// Строки внутри схемы копируются в члены RCol ровно по одному разу
    void initialize(const ITableRepository::ColumnSchema& schema);

    /// Обновление локального кеша без обращения к плагину.
    /// Вызывается из RtabController после записи через ITableRepository.
    void updateCachedProperty(FieldProperties prop, const std::string& val);

    void invertDirectCodeStatus() { m_directcode = !m_directcode; }
    void setNameRef(const std::string& v) { m_cached_nameref = v; }
    void setHidden(bool v)                { m_hidden = v; }

    // ── Геттеры — только из кеша, возвращают const& ──────────────────────
    long               getIndex()      const { return m_index; }
    bool               isDirectCode()  const { return m_directcode; }
    bool               isHidden()      const { return m_hidden; }
    enComPropTT        getComPropTT()  const { return m_com_prop_tt; }
    _en_data           getEnData()     const { return m_en_data; }

    const std::string& getTableName()  const { return m_table_name; }
    const std::string& getColName()    const { return m_colName; }
    const std::string& getTitle()      const { return m_cached_title; }
    const std::string& getDesc()       const { return m_cached_desc; }
    const std::string& getUnit()       const { return m_cached_unit; }
    const std::string& getWidth()      const { return m_cached_width; }
    const std::string& getPrec()       const { return m_cached_prec; }
    const std::string& getExpr()       const { return m_cached_expression; }
    const std::string& getNameRef()    const { return m_cached_nameref; }
    const std::string& getAfor()       const { return m_cached_afor; }
    const std::string& getFF()         const { return m_cached_ff; }
    const std::string& getMin()        const { return m_cached_min; }
    const std::string& getMax()        const { return m_cached_max; }
    const std::string& getScale()      const { return m_cached_scale; }
    const std::string& getCache()      const { return m_cached_cache; }

private:
    // ── Идентификация ─────────────────────────────────────────────────────
    std::string m_colName;			///< внутреннее имя колонки (otv)
    std::string m_table_name;		///< имя таблицы-владельца (Узлы)
    long        m_index      = -1;	///< порядковый индекс колонки в таблице плагина

    // ── Кешированные метаданные (обновляются в setMeta) ───────────────────
    std::string m_cached_title;		///< Название (dV)
    std::string m_cached_desc;		///< Описание (Отклонение напряжения от номинального)
    std::string m_cached_unit;		///< Ед. измерения (%)
    std::string m_cached_width;		///<
    std::string m_cached_prec;		///<
    std::string m_cached_expression;///<if(sta=0) (vras-uhom)/uhom*100:0
    std::string m_cached_nameref;  	///< Перечисление/ссылка на справочник ("node[na]" / "ti_prv.Name.Num")
    std::string m_cached_afor;		///<
    std::string m_cached_ff;		///< формула активна
    std::string m_cached_min;		///<
    std::string m_cached_max;		///<
    std::string m_cached_scale;		///< Масштаб
    std::string m_cached_cache;		///< Кэш

    // ── Семантика типа ────────────────────────────────────────────────────
    enComPropTT m_com_prop_tt = enComPropTT::COM_PR_INT; ///< семантический тип колонки (ENUM, REAL, INT, …)
    _en_data    m_en_data     = _en_data::DATA_ERR; 				///<C++-тип данных (int/double/bool/string)

    // ── Состояние UI ──────────────────────────────────────────────────────
    bool m_directcode = false; ///< режим ввода: true = число, false = строка из справочника
    bool m_hidden     = true; ///<скрыта ли колонка в UI (не входит в форму)
};
