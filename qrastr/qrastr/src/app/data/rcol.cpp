#include "rcol.h"

#include <astra/IPlainRastrWrappers.h>
#include "qastra.h"

void RCol::setMeta(QAstra* pqastra){

    IRastrTablesPtr  tables  { pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(m_table_name) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(m_index) };

    auto prop = [&](FieldProperties p) -> std::string {
        return IRastrPayload(
                   IRastrVariantPtr(col_ptr->Property(p))->String()
                   ).Value();
    };

    auto setTitle = [&]() -> std::string {
        long indx = IRastrPayload{columns->FindIndex(m_colName)}.Value();
        if (indx < 0)
        {
            std::string _tmp = "->no column!";
            return _tmp.append(m_colName);
        }
        IRastrColumnPtr col_ptr{ columns->Item(m_colName) };
        return IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Title))->String()).Value();
    };

    // Кешируем всё за один обход к плагину
    m_cached_title   	= setTitle();
    m_cached_desc    	= prop(FieldProperties::Description);
    m_cached_unit    	= prop(FieldProperties::Unit);
    m_cached_width   	= prop(FieldProperties::Width);
    m_cached_prec    	= prop(FieldProperties::Precision);
    m_cached_expression = prop(FieldProperties::Expression);
    m_cached_nameref 	= prop(FieldProperties::NameRef);
    m_cached_afor 		= prop(FieldProperties::AFOR);
    m_cached_ff 		= prop(FieldProperties::IsActiveFormula);
    m_cached_min 		= prop(FieldProperties::Min);
    m_cached_max 		= prop(FieldProperties::Max);
    m_cached_scale 		= prop(FieldProperties::Scale);
    m_cached_cache 		= prop(FieldProperties::Cache);

    const int n_type = std::stoi(prop(FieldProperties::Type));
    m_com_prop_tt = static_cast<enComPropTT>(n_type);

    switch (m_com_prop_tt) {
    case enComPropTT::COM_PR_BOOL:
        m_en_data = _en_data::DATA_BOOL; break;
    case enComPropTT::COM_PR_INT:
    case enComPropTT::COM_PR_ENUM:
    case enComPropTT::COM_PR_ENPIC:
    case enComPropTT::COM_PR_COLOR:
    case enComPropTT::COM_PR_SUPERENUM:
    case enComPropTT::COM_PR_TIME:
    case enComPropTT::COM_PR_HEX:
        m_en_data = _en_data::DATA_INT; break;
    case enComPropTT::COM_PR_REAL:
        m_en_data = _en_data::DATA_DBL; break;
    case enComPropTT::COM_PR_STRING:
        m_en_data = _en_data::DATA_STR; break;
    default:
        m_en_data = _en_data::DATA_ERR; break;
    }
}

void RCol::initialize(const std::string& col_name,
                      const std::string& table_name,
                      long index){

    m_colName   = col_name;
    m_table_name = table_name;
    m_index      = index;
}

std::string RCol::set_prec(QAstra* pqastra, const std::string& str_prec)
{
    // Все промежуточные объекты живут до конца функции
    IRastrTablesPtr  tables  { pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(m_table_name) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(m_colName) };

    IRastrResultVerify{col_ptr->SetProperty(FieldProperties::Precision, str_prec)};
    m_cached_prec = str_prec;
    return str_prec;
}

std::string RCol::set_prop(QAstra* pqastra,
                           FieldProperties prop,
                           const std::string& val)
{
    IRastrTablesPtr  tables  { pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(m_table_name) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(m_colName) };

    IRastrResultVerify{col_ptr->SetProperty(prop, val)};

    // Обновляем локальный кеш немедленно.
    // handleChangeTable придёт после SetLockEvent(false) и пересоздаст
    // RData целиком — но до этого момента кеш должен быть актуален.
    switch (prop) {
    case FieldProperties::Title:       m_cached_title      = val; break;
    case FieldProperties::Description: m_cached_desc       = val; break;
    case FieldProperties::Width:       m_cached_width      = val; break;
    case FieldProperties::Expression:  m_cached_expression = val; break;
    default: break;
    }
    return val;
}

void RCol::calc(QAstra* pqastra,
                const std::string& expression,
                const std::string& selection) const
{
    IRastrTablesPtr  tables  { pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(m_table_name) };
    IRastrResultVerify(table->SetSelection(selection));
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(m_colName) };
    IRastrResultVerify(col_ptr->Calculate(expression));
}