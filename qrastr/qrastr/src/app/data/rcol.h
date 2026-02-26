#pragma once

#include "astra_shared.h"
#include "qastra.h"
#include "json.hpp"

using WrapperExceptionType = std::runtime_error;


#include <astra/IPlainRastrWrappers.h>

using _vt = std::variant< bool,long, double, std::string >;
typedef std::vector< _vt > _col_data ;


/**
 * @brief Метаданные и вспомогательные операции для одной колонки таблицы Rastr.
 * Каждый геттер (name(), title(), desc(), …) делает отдельный вызов в плагин.
 */
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
    ///@todo следует реализовать
    void setMeta(const nlohmann::json& j_meta_in){}
    /**
     * Читает тип колонки из плагина и заполняет en_data_ / com_prop_tt.
     * Вызывается один раз при построении RData::Initialize().
     */
    void setMeta(QAstra* _pqastra);

    std::string name() const;
    std::string Type() const;
    std::string width() const;
    std::string title() const;
    std::string desc() const;
    std::string prec() const;
    std::string set_prec(std::string str_prec) const;
    std::string set_prop(FieldProperties _prop, std::string _str_prop) const;
    std::string expr() const;
    std::string AFOR() const;
    std::string IsActiveFormula() const;
    std::string NameRef() const;

    std::string Min() const;
    std::string Max() const;
    std::string Scale() const;
    std::string Cache() const;
    std::string unit() const;

    std::string Prop(FieldProperties _Prop) const;

    void calc(std::string expression , std::string selection) const;

    enComPropTT    com_prop_tt; ///< семантический тип колонки (ENUM, REAL, INT, …)
    std::string    str_name_; ///<внутреннее имя колонки
    std::string    table_name_; ///<имя таблицы-владельца
    _en_data       en_data_; ///<C++-тип данных (int/double/bool/string)
    std::string    title_;
    std::string    nameref_; ///< строка-ссылка на справочник ("node[na]" / "ti_prv.Name.Num")
    bool directcode; ///< режим ввода: true = число, false = строка из справочника
    long    index; ///< порядковый индекс колонки в таблице плагина
    bool hidden ; ///<скрыта ли колонка в UI (не входит в форму)
private:
    nlohmann::json j_meta_;
    QAstra* pqastra_;

};
