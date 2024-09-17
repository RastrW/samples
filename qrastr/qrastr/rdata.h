#ifndef RDATA_H
#define RDATA_H

#include "rastrhlp.h"
#include "astra_exp.h"
#include "astra_shared.h"
#include "UIForms.h"
#include "qastra.h"

using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"

//typedef std::variant< int, double, std::string >  _vt ;
typedef std::variant< bool,long, double, std::string >  _vt ;
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
    }
    virtual ~RCol() = default;
    void setMeta(const nlohmann::json& j_meta_in){
        j_meta_ = j_meta_in;
        en_data_ = _en_data::DATA_ERR;
        const std::string str_Type = j_meta_["Type"];
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
        const std::string str_name = j_meta_["Name"];
        return str_name;
    }
    std::string Type() const{
        const std::string str_Type = j_meta_["Type"];
        return str_Type;
    }
    std::string width() const{
        const std::string str_width = j_meta_["Width"];
        return str_width;
    }
    std::string prec() const{
        const std::string str_prec = j_meta_["Precision"];
        return str_prec;
    }
    std::string title() const{
        const std::string str_title = j_meta_["Title"];
        return str_title;
    }
    std::string expr() const{
        const std::string str_expr = j_meta_["Expression"];
        return str_expr;
    }
    std::string AFOR() const{
        const std::string str_AFOR = j_meta_["AFOR"];
        return str_AFOR;
    }
    std::string IsActiveFormula() const{
        const std::string str_IsActiveFormula = j_meta_["IsActiveFormula"];
        return str_IsActiveFormula;
    }
    std::string NameRef() const{
        const std::string str_NameRef = j_meta_["NameRef"];
        return str_NameRef;
    }
    std::string desc() const{
        const std::string str_desc = j_meta_["Description"];
        return str_desc;
    }
    std::string Min() const{
        const std::string str_Min = j_meta_["Min"];
        return str_Min;
    }
    std::string Max() const{
        const std::string str_Max = j_meta_["Max"];
        return str_Max;
    }
    std::string Scale() const{
        const std::string str_Scale = j_meta_["Scale"];
        return str_Scale;
    }
    std::string Cache() const{
        const std::string str_Cache = j_meta_["Cache"];
        return str_Cache;
    }
    std::string unit() const{
        const std::string str_unit = j_meta_["Unit"];
        return str_unit;
    }

    enComPropTT com_prop_tt;
    std::string    str_name_;
    _en_data       en_data_;
    std::string nameref;
    long    index;
private:
    nlohmann::json j_meta_;
};// class RCol

class RData
    : public std::vector<RCol> {
public:
    RData()
    {
    }
    RData(const _idRastr _id_rastr, std::string _t_name)
    {
        id_rastr_ = _id_rastr;
        t_name_ = _t_name;
    }
    void SetNumRows(long n_new_num_rows){
        n_num_rows_ = n_new_num_rows;
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




    void Initialize(CUIForm _form);
    void Initialize(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form);// old
    // Заполняем через плоскую dll через запрос GetJSON
    void populate();
    void populate(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form);  // old Заполнить данными

    //Заполняем через IRastrPlain
    void populate_qastra(QAstra* _pqastra);

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

    _idRastr id_rastr_ = 0;
    std::string t_name_ = "";
    std::string t_title_ = "";
    std::string str_cols_ = "";                     // строка имен столбцов ex: "ny,pn,qn,vras"
    nlohmann::json j_metas_;                        // Мета информация о таблице (шаблон)
private:
    const int SIZE_STR_BUF = 500'000'000;
    long n_num_rows_ = 0;

};// class RData


#endif // RDATA_H
