#include "rcol.h"

void RCol::setMeta(QAstra* _pqastra){
    pqastra_ = _pqastra;
    IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_index) };

    m_en_data = _en_data::DATA_ERR;

    const std::string str_Type = Type();
    int n_type = std::stoi(str_Type);
    m_com_prop_tt = static_cast<enComPropTT>(n_type);
    switch(m_com_prop_tt){
    case enComPropTT::COM_PR_BOOL	   : //= 3,
        m_en_data = _en_data::DATA_BOOL;
        break;
    case enComPropTT::COM_PR_INT	   : //= 0,
    case enComPropTT::COM_PR_ENUM	   : //= 4,
    case enComPropTT::COM_PR_ENPIC	   : //= 5,
    case enComPropTT::COM_PR_COLOR	   : //= 6,
    case enComPropTT::COM_PR_SUPERENUM : //= 7,
    case enComPropTT::COM_PR_TIME	   : //= 8,
    case enComPropTT::COM_PR_HEX	   : //= 9
        m_en_data = _en_data::DATA_INT;
        break;
    case enComPropTT::COM_PR_REAL	   : //= 1,
        m_en_data = _en_data::DATA_DBL;
        break;
    case enComPropTT::COM_PR_STRING	   : //= 2,
        m_en_data = _en_data::DATA_STR;
        break;
    }
}

std::string RCol::name() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_name = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Name))->String()).Value();
    return str_name;
}

std::string RCol::Type() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_Type = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Type))->String()).Value();
    return str_Type;
}

std::string RCol::width() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_width = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Width))->String()).Value();
    return str_width;
}

std::string RCol::title() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    long indx = IRastrPayload{columns->FindIndex(m_str_name)}.Value();
    if (indx < 0)
    {
        qDebug()<<"rdata->title(): "<<QString::fromStdString(m_str_name)<< "column not found! ";
        std::string _tmp = "->no column!";
        return _tmp.append(m_str_name);
    }
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_title = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Title))->String()).Value();
    return str_title;
}

std::string RCol::desc() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_desc = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Description))->String()).Value();
    return str_desc;
}

std::string RCol::prec() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_prec = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Precision))->String()).Value();
    return str_prec;
}

std::string RCol::set_prec(std::string str_prec) const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    IRastrResultVerify{col_ptr->SetProperty(FieldProperties::Precision,str_prec)};
    return str_prec;
}

std::string RCol::set_prop(FieldProperties _prop, std::string _str_prop) const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    IRastrResultVerify{col_ptr->SetProperty(_prop,_str_prop)};
    return _str_prop;
}

std::string RCol::expr() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_expr = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Expression))->String()).Value();
    return str_expr;
}

std::string RCol::AFOR() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_AFOR = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::AFOR))->String()).Value();
    return str_AFOR;
}

std::string RCol::IsActiveFormula() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_IsActiveFormula = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::IsActiveFormula))->String()).Value();
    return str_IsActiveFormula;
}

std::string RCol::NameRef() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_NameRef = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::NameRef))->String()).Value();
    return str_NameRef;
}

std::string RCol::Min() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_Min = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Min))->String()).Value();
    return str_Min;
}

std::string RCol::Max() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_Max = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Max))->String()).Value();
    return str_Max;
}

std::string RCol::Scale() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_Scale = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Scale))->String()).Value();
    return str_Scale;
}

std::string RCol::Cache() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_Cache = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Cache))->String()).Value();
    return str_Cache;
}

std::string RCol::unit() const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_unit = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Unit))->String()).Value();
    return str_unit;
}

std::string RCol::Prop(FieldProperties _Prop) const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    std::string str_prop = IRastrPayload(IRastrVariantPtr(col_ptr->Property(_Prop))->String()).Value();
    return str_prop;
}

void RCol::calc(std::string expression , std::string selection) const{
    IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(m_table_name) };
    IRastrResultVerify(table->SetSelection(selection));
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col_ptr{ columns->Item(m_str_name) };
    IRastrResultVerify( col_ptr->Calculate(expression));
}
