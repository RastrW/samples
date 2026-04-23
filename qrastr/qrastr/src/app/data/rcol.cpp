#include "rcol.h"

#include <astra/IPlainRastrWrappers.h>

void RCol::initialize(const ITableRepository::ColumnSchema& schema)
{
    // Каждая строка копируется ровно один раз — из схемы в член класса.
    // Раньше то же самое делал setMeta() через N вызовов плагина.
    // Сам объект schema не копируется — передан по const&.
    m_colName           = schema.name;
    m_table_name        = schema.tableName;
    m_index             = schema.index;
    m_cached_title      = schema.title;
    m_cached_desc       = schema.description;
    m_cached_unit       = schema.unit;
    m_cached_width      = schema.width;
    m_cached_prec       = schema.prec;
    m_cached_expression = schema.expression;
    m_cached_nameref    = schema.nameRef;
    m_cached_afor       = schema.afor;
    m_cached_ff         = schema.ff;
    m_cached_min        = schema.min;
    m_cached_max        = schema.max;
    m_cached_scale      = schema.scale;
    m_cached_cache      = schema.cache;
    m_com_prop_tt       = schema.comPropTT;

    switch (m_com_prop_tt) {
    case enComPropTT::COM_PR_BOOL:
        m_en_data = _en_data::DATA_BOOL; break;
    case enComPropTT::COM_PR_REAL:
        m_en_data = _en_data::DATA_DBL;  break;
    case enComPropTT::COM_PR_STRING:
        m_en_data = _en_data::DATA_STR;  break;
    case enComPropTT::COM_PR_INT:
    case enComPropTT::COM_PR_ENUM:
    case enComPropTT::COM_PR_ENPIC:
    case enComPropTT::COM_PR_COLOR:
    case enComPropTT::COM_PR_SUPERENUM:
    case enComPropTT::COM_PR_TIME:
    case enComPropTT::COM_PR_HEX:
        m_en_data = _en_data::DATA_INT;  break;
    default:
        m_en_data = _en_data::DATA_ERR;  break;
    }
}

void RCol::updateCachedProperty(FieldProperties prop, const std::string& val)
{
    // Вызывается из RtabController после успешной записи в плагин через репозиторий.
    // val передаётся по const& — копирование только в тот член, который меняется.
    switch (prop) {
    case FieldProperties::Title:       m_cached_title      = val; break;
    case FieldProperties::Description: m_cached_desc       = val; break;
    case FieldProperties::Width:       m_cached_width      = val; break;
    case FieldProperties::Precision:   m_cached_prec       = val; break;
    case FieldProperties::Expression:  m_cached_expression = val; break;
    case FieldProperties::NameRef:     m_cached_nameref    = val; break;
    default: break;
    }
}
