#pragma once

#include "astra_shared.h"
#include "qastra.h"
#include "json.hpp"

using WrapperExceptionType = std::runtime_error;


#include <astra/IPlainRastrWrappers.h>

/**
 * @brief Метаданные и вспомогательные операции для одной колонки таблицы Rastr.
 * Каждый геттер (name(), title(), desc(), …) делает отдельный вызов в плагин.
 * @todo подумать о том, чтобы кешировать данные при создании объекта, чтобы скоратить количество вызовов в плагин
 * Но необходимо понимать, какие из них неизменяемы, а, если нет, то в каких случаях изменяются
 */
class RCol{
public:
    enum _en_data{ // in _col_data
        DATA_ERR =  -1,
        DATA_BOOL=   0,
        DATA_INT =   1,
        DATA_DBL =   2,
        DATA_STR =   3
    };
    RCol() = default;

    virtual ~RCol() = default;
    ///@todo следует реализовать
    void setMeta(const nlohmann::json& j_meta_in){}
    /**
     * Читает тип колонки из плагина и заполняет en_data_ / com_prop_tt.
     * Вызывается один раз при построении создании Initialize().
     */
    void setMeta(QAstra* _pqastra);
    void calc(const std::string& expression ,
              const std::string& selection) const;

    ///Получить значение
    long getIndex() const{ return m_index; }
    bool isDirectCode() const{ return m_directcode; }
    bool isHidden() const{ return m_hidden; }
    enComPropTT getComPropTT() const{return m_com_prop_tt; }
    _en_data getEnData() const{ return m_en_data; }
    const std::string& getNameRef() const{ return m_nameref; }
    const std::string& getTableName() const{ return m_table_name; }
    const std::string& getStrName() const{ return m_str_name; }
    const std::string& getTitle() const{ return m_title;}

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

    ///Установить значение
    void invertDirectCodeStatus(){
        m_directcode = !m_directcode;
    }
    void initialize (const std::string& str_name,
                    const std::string& table_name,
                    const std::string& title,
                    long index){
        m_str_name = str_name;
        m_table_name = table_name;
        m_title = title;
        m_index = index;
    }
    void setNameRef(const std::string& nameRef){ m_nameref = nameRef; }
    void setHidden(bool hidden){ m_hidden = hidden; }
private:
    nlohmann::json j_meta_;
    QAstra* pqastra_;

    enComPropTT    m_com_prop_tt; ///< семантический тип колонки (ENUM, REAL, INT, …)
    std::string    m_str_name; ///<внутреннее имя колонки
    std::string    m_table_name; ///<имя таблицы-владельца
    _en_data       m_en_data; ///<C++-тип данных (int/double/bool/string)
    std::string    m_title;
    std::string    m_nameref; ///< строка-ссылка на справочник ("node[na]" / "ti_prv.Name.Num")
    bool m_directcode {false}; ///< режим ввода: true = число, false = строка из справочника
    long    m_index; ///< порядковый индекс колонки в таблице плагина
    bool m_hidden {true}; ///<скрыта ли колонка в UI (не входит в форму)
};
