#ifndef RDATA_H
#define RDATA_H

#include "rastrhlp.h"
#include "astra_exp.h"
#include "astra_shared.h"
#include "UIForms.h"
#include "qastra.h"
#include "iostream"
//#include "stringutils.h";

using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"

//typedef std::variant< int, double, std::string >  _vt ;
typedef std::variant< bool,long, double, std::string >  _vt ;
typedef std::vector< _vt > _col_data ;
typedef std::vector<std::string> _vstr;

template<typename T>
class DataBlock : public IRastrDataBlock<T>
{
protected:
    T* Block_ = nullptr;
    long Rows_ = 0;
    long Columns_ = 0;
    long* pIndiciesChanged_ = nullptr;
    std::list<std::string> ColumnTitles_;
    std::vector<long> vChangedIndices_;

public:
    IPlainRastrRetCode SetBlockSize(long Rows, long Columns) noexcept override
    {
        if (Block_ != nullptr)
            delete[] Block_;

        ColumnTitles_.clear();

        Rows_ = Columns_ = 0;
        Block_ = new T[Rows * Columns];
        if (Block_ != nullptr)
        {
            Rows_ = Rows;
            Columns_ = Columns;
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }
    ~DataBlock()
    {
        delete[] Block_;
    }

    // отдаем SparseDataBlock в сервер

    // Говорим сколько вообще значений в DataBlock
    long ValuesAvailable() const noexcept override
    {
        return Rows_ * Columns_;
    }

    // Возвращаем значение в диапазоне [0; ValuesAvailable)
    // в структуре (строка, столбец, const* значение, код возврата)
    IRastrDataBlock<T>::ValueReturn Value(long Index) const noexcept override
    {
        // здесь демо возврата просто прямоугольной таблицы
        // если есть строки / столбцы / инфа о дефолтах внутри датаблока - отдаем из списка
        // по коду возврата можно закенселить, если хочется.
        //return { Index / Columns(), Index % Columns(), Block_ + Index, IPlainRastrRetCode::Ok };
        // если вернуть из Value NotImplemented - будет предпринята попытка
        // получить dense блок - см. ниже
        return IRastrDataBlock<T>::Value(Index);
    }

    // Ну а если блок dense - то отдаем

    const T* Data() const override { return Block_; }
    T* Data() override { return Block_; }
    long Rows() const override { return Rows_; }
    long Columns() const override { return Columns_; }
    const long* ChangedIndices() const override { return vChangedIndices_.data(); }
    const size_t ChangedIndicesCount() const override { return vChangedIndices_.size(); }


    // и забираем просто из прямоугольной таблицы


    IPlainRastrRetCode Emplace(long Row, long Column, const T& Value) noexcept override
    {
        if (Row >= 0 && Row < Rows_ && Column >= 0 && Column < Columns_)
        {
            Block_[Row * Columns_ + Column] = Value;
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }
    IPlainRastrRetCode EmplaceSaveIndChange(long Row, long Column, const T& Value) noexcept override
    {
        if (Emplace(Row, Column, Value) != IPlainRastrRetCode::Failed)
        {
            vChangedIndices_.push_back(Row * Columns_ + Column);
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }

    IPlainRastrRetCode MapColumn(const std::string_view Name, long DataBlockIndex) const noexcept override
    {
        const_cast<DataBlock*>(this)->ColumnTitles_.emplace_back(Name);
        return IPlainRastrRetCode::Ok;
    }

    struct ToString {
        std::string operator()(std::monostate) { return { "def" }; }
        std::string operator()(const long& value) { return std::to_string(value); }
        std::string operator()(const uint64_t& value) { return std::to_string(value); }
        std::string operator()(const double& value) { return std::to_string(value); }
        std::string operator()(const bool& value) { return value ? "1" : "0"; }
        std::string operator()(const std::string& value) { return value; }
    };
    struct ToVal {
        std::string operator()(std::monostate) { return { "def" }; }
        long operator()(const long& value) { return value; }
        std::string operator()(const uint64_t& value) { return std::to_string(value); }
        std::string operator()(const double& value) { return std::to_string(value); }
        std::string operator()(const bool& value) { return value ? "1" : "0"; }
        std::string operator()(const std::string& value) { return value; }
    };

    void Dump()
    {
        for (const auto& ColumnTitle : ColumnTitles_)
            std::cout << ColumnTitle << ";";
        std::cout << std::endl;
        for (long row = 0; row < Rows(); row++)
        {
            for (long column = 0; column < Columns(); column++)
                std::cout << StringValue(Data()[row * Columns() + column]) << ";";
            std::cout << std::endl;
        }
    }
    void QDump()
    {
        std::string str_cols = "";
        for (const auto& ColumnTitle : ColumnTitles_)
        {
            str_cols.append(ColumnTitle);
            str_cols.append(";");
        }
        qDebug() << str_cols;
        for (long row = 0; row < Rows(); row++)
        {
            std::string str_out = "";
            for (long column = 0; column < Columns(); column++)
            {
                str_out.append(StringValue(Data()[row * Columns() + column]));
                str_out.append(";");
            }
            qDebug() << str_out << ";";
        }
    }

protected:
    std::string StringValue(const double& Value)
    {
        return std::to_string(Value);
    }

    std::string StringValue(const FieldVariantData& Value)
    {
        return std::visit(ToString(), Value);
    }
};

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
   /* RCol(const RCol &col)
    {
        //pcol_ptr2 = col.pcol_ptr2;

    }*/
    virtual ~RCol() = default;
    void setMeta(const nlohmann::json& j_meta_in){

    }

    //void setMeta(IRastrColumnPtr* _pcol_ptr){
    void setMeta(QAstra* _pqastra){
        pqastra_ = _pqastra;
        IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(table_name_) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col_ptr{ columns->Item(str_name_) };

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
        col_ptr->SetProperty(FieldProperties::Precision,str_prec);
        return str_prec;
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


    enComPropTT com_prop_tt;
    std::string    str_name_;
    std::string    table_name_;
    _en_data       en_data_;
    //std::string nameref;
    //std::string title_;
    long    index;
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




    void Initialize(CUIForm _form, QAstra* _pqastra);
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
    QAstra* pqastra_;
    std::string t_name_ = "";
    std::string t_title_ = "";
    std::string str_cols_ = "";                     // строка имен столбцов ex: "ny,pn,qn,vras"
    //nlohmann::json j_metas_;                        // Мета информация о таблице (шаблон)
    DataBlock<FieldVariantData> nparray_;
private:
    const int SIZE_STR_BUF = 500'000'000;
    long n_num_rows_ = 0;

};// class RData


#endif // RDATA_H
