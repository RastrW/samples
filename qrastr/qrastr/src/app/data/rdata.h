#pragma once

#include "astra_shared.h"
#include "astra_headers/UIForms.h"
#include "qastra.h"
#include "rtablesdatamanager.h"
using WrapperExceptionType = std::runtime_error;
#include "QDataBlocks.h"

#include <astra/IPlainRastrWrappers.h>

using _vt = std::variant< bool,long, double, std::string >;
typedef std::vector< _vt > _col_data ;
typedef std::vector<std::string> _vstr;

class RCol
    : public _col_data
{
public:
    enum _en_data{ // in _col_data
        DATA_ERR =  -1,
        DATA_BOOL=   0,
        DATA_INT =   1,
        DATA_DBL =   2,
        DATA_STR =   3
    };
    template <typename... Args>
    RCol(Args&&... args)
        : _col_data{args...} {
        hidden = true;
        directcode = false;
    }

    virtual ~RCol() = default;
    void setMeta(const nlohmann::json& j_meta_in){

    }

    //void setMeta(IRastrColumnPtr* _pcol_ptr){
    void setMeta(QAstra* _pqastra){
        pqastra_ = _pqastra;
        IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        //IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        IRastrColumnPtr col_ptr{ columns->Item(index) };

        //pcol_ptr = _pcol_ptr;
        //j_meta_ = j_meta_in;
        en_data_ = _en_data::DATA_ERR;
        //const std::string str_Type = j_meta_["Type"];
        const std::string str_Type = Type();
        int n_type = std::stoi(str_Type);
        com_prop_tt = static_cast<enComPropTT>(n_type);
        switch(com_prop_tt){
        case enComPropTT::COM_PR_BOOL	   : //= 3,
            en_data_ = _en_data::DATA_BOOL;
            break;
        case enComPropTT::COM_PR_INT	   : //= 0,
        case enComPropTT::COM_PR_ENUM	   : //= 4,
        case enComPropTT::COM_PR_ENPIC	   : //= 5,
        case enComPropTT::COM_PR_COLOR	   : //= 6,
        case enComPropTT::COM_PR_SUPERENUM : //= 7,
        case enComPropTT::COM_PR_TIME	   : //= 8,
        case enComPropTT::COM_PR_HEX	   : //= 9
            en_data_ = _en_data::DATA_INT;
            break;
        case enComPropTT::COM_PR_REAL	   : //= 1,
            en_data_ = _en_data::DATA_DBL;
            break;
        case enComPropTT::COM_PR_STRING	   : //= 2,
            en_data_ = _en_data::DATA_STR;
            break;
        }
    }
    std::string name() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_name = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Name))->String()).Value();
        return str_name;
    }

    std::string Type() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_Type = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Type))->String()).Value();
        return str_Type;
    }

    std::string width() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_width = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Width))->String()).Value();
        return str_width;
    }

    std::string title() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_title = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Title))->String()).Value();
        return str_title;
    }
    std::string desc() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_desc = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Description))->String()).Value();
        return str_desc;
    }

    std::string prec() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_prec = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Precision))->String()).Value();
        return str_prec;
    }
    std::string set_prec(std::string str_prec) const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        IRastrResultVerify{col_ptr->SetProperty(FieldProperties::Precision,str_prec)};
        return str_prec;
    }
    std::string set_prop(FieldProperties _prop, std::string _str_prop) const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        IRastrResultVerify{col_ptr->SetProperty(_prop,_str_prop)};
        return _str_prop;
    }
    std::string expr() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_expr = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Expression))->String()).Value();
        return str_expr;
    }

    std::string AFOR() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_AFOR = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::AFOR))->String()).Value();
        return str_AFOR;
    }
    std::string IsActiveFormula() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_IsActiveFormula = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::IsActiveFormula))->String()).Value();
        return str_IsActiveFormula;
    }
    std::string NameRef() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_NameRef = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::NameRef))->String()).Value();
        return str_NameRef;
    }

    std::string Min() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_Min = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Min))->String()).Value();
        return str_Min;
    }
    std::string Max() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_Max = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Max))->String()).Value();
        return str_Max;
    }
    std::string Scale() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_Scale = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Scale))->String()).Value();
        return str_Scale;
    }
    std::string Cache() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_Cache = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Cache))->String()).Value();
        return str_Cache;
    }

    std::string unit() const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_unit = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::Unit))->String()).Value();
        return str_unit;
    }
    std::string Prop(FieldProperties _Prop) const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        std::string str_prop = IRastrPayload(IRastrVariantPtr(col_ptr->Property(_Prop))->String()).Value();
        return str_prop;
    }

    void calc(std::string expression , std::string selection) const{
        IRastrTablesPtr tablesx{ pqastra_->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrResultVerify(table->SetSelection(selection));
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };
        IRastrResultVerify( col_ptr->Calculate(expression));
    }

    enComPropTT    com_prop_tt;
    std::string    str_name_;
    std::string    table_name_;
    _en_data       en_data_;
    std::string    title_;
    std::string    nameref_;
    bool directcode;                // Прямой ввод кода
    long    index;
    bool hidden ;
private:
    nlohmann::json j_meta_;
    //IRastrColumnPtr col_ptr;
    QAstra* pqastra_;

};// class RCol

class RData
    : public std::vector<RCol> {
public:
    RData()
    {
    }
    RData(QAstra* _pqastra, std::string _t_name)
    {
        pqastra_ = _pqastra;
        t_name_ = _t_name;
        //nparray_ = new  MyDenseDataBlock<FieldVariantData>(pqastra_);
    }

    int AddCol(const RCol& rcol){
        //      if(rcol.size()!=n_num_rows_)
        //          return -1;
        emplace_back(rcol);
        //qDebug() << rcol.name() << "->" << rcol.NameRef();
        //push_back(rcol);
        return static_cast<int>(size());
    }

    int AddRow(int index = -1);
    int RemoveRDMRow(int index = -1);

    void Initialize(CUIForm _form, QAstra* _pqastra);
    void Initialize(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form);// old
    // Заполняем через плоскую dll через запрос GetJSON
    void populate();
    void populate(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form);  // old Заполнить данными

    //Заполняем через IRastrPlain
    void populate_qastra(QAstra* _pqastra,RTablesDataManager* _pRTDM);
    std::string get_cols(bool visible = true);

    void clear_data();                                                                          // Удалить данные (стуктура остается)

    std::string getCommaSeparatedFieldNames(){
        std::string str_tmp;
        for( const RCol& col_data : *this ) {
            str_tmp += col_data.str_name_;
            str_tmp += ",";
        }
        if(str_tmp.length()>0){
            str_tmp.erase(str_tmp.length()-1);
        }
        return str_tmp;
    }
    void Trace() const {
        for(const RCol& col : *this){
            qDebug() << " col: " << col.str_name_.c_str();
            for(const _col_data::value_type& cdata : col ){
                switch(col.en_data_){
                case RCol::_en_data::DATA_INT :
                    qDebug()<<"cdata : "<< std::get<long>(cdata);
                    break;
                case RCol::_en_data::DATA_DBL :
                    qDebug()<<"cdata : "<< std::get<double>(cdata);
                    break;
                case RCol::_en_data::DATA_STR :
                    qDebug()<<"cdata : "<< std::get<std::string>(cdata).c_str();
                    break;
                default:
                    qDebug()<<"cdata : unknown!! ";
                    break;
                }
                //qDebug()<<"cdata : "<< std::to_string(cdata).c_str();
            }
        }
    }

   // _idRastr id_rastr_ = 0;
    QAstra* pqastra_;
    std::string t_name_ = "";
    std::string t_title_ = "";
    std::string str_cols_ = "";                     // строка имен столбцов ex: "ny,pn,qn,vras"
    std::vector<std::string> vCols_;                // вектор имен столбцов
    std::shared_ptr<QDataBlock> pnparray_;
    std::map<std::string, int> mCols_;
private:

};// class RData
