#ifndef RDATA_H
#define RDATA_H
#pragma once

#include "rastrhlp.h"
//#include "astra_exp.h"
#include "astra_shared.h"
//#include "UIForms.h"
#include <astra\UIForms.h>
#include "qastra.h"
#include "rtablesdatamanager.h"
//#include "iostream"
//#include "stringutils.h";
using WrapperExceptionType = std::runtime_error;
//#include "IDataBlocksWrappers.h"
#include "QDataBlocks.h"

//using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>
//#include "IDataBlocksWrappers.h"


using _vt = std::variant< bool,long, double, std::string >;
typedef std::vector< _vt > _col_data ;
typedef std::vector<std::string> _vstr;

/*
struct ToString {
    std::string operator()(std::monostate) { return { "def" }; }
    std::string operator()(const long& value) { return std::to_string(value); }
    std::string operator()(const uint64_t& value) { return std::to_string(value); }
    std::string operator()(const double& value) { return std::to_string(value); }
    std::string operator()(const bool& value) { return value ? "1" : "0"; }
    std::string operator()(const std::string& value) { return value; }
};
template<typename T>
struct VariantToString
{
    static std::string String(const T& Value)
    {
        return std::to_string(Value);
    }
};


template<typename T>
struct MapFieldVariantType
{
    static eFieldVariantType Type() noexcept;
    static const std::string_view VerbalType() noexcept;
};

template<> eFieldVariantType MapFieldVariantType<FieldVariantData>::Type() noexcept { return eFieldVariantType::Monostate; }
template<> eFieldVariantType MapFieldVariantType<double>::Type() noexcept { return eFieldVariantType::Double; }
template<> eFieldVariantType MapFieldVariantType<bool>::Type() noexcept { return eFieldVariantType::Bool; }
template<> eFieldVariantType MapFieldVariantType<std::string>::Type() noexcept { return eFieldVariantType::String; }
template<> eFieldVariantType MapFieldVariantType<long>::Type() noexcept { return eFieldVariantType::Long; }
template<> eFieldVariantType MapFieldVariantType<uint64_t>::Type() noexcept { return eFieldVariantType::Uint64; }
template<> const std::string_view MapFieldVariantType<FieldVariantData>::VerbalType() noexcept { return "monostate"; }
template<> const std::string_view MapFieldVariantType<double>::VerbalType() noexcept { return "double"; }
template<> const std::string_view MapFieldVariantType<bool>::VerbalType() noexcept { return "bool"; }
template<> const std::string_view MapFieldVariantType<std::string>::VerbalType() noexcept { return "string"; }
template<> const std::string_view MapFieldVariantType<long>::VerbalType() noexcept { return "long"; }
template<> const std::string_view MapFieldVariantType<uint64_t>::VerbalType() noexcept { return "uint64"; }

template<typename T>
struct VariantToString
{
    static std::string String(const T& Value)
    {
        return std::to_string(Value);
    }
};

template<> std::string VariantToString<FieldVariantData>::String(const FieldVariantData& Value)
{
    return std::visit(ToString(), Value);
}
template<> std::string VariantToString<std::string>::String(const std::string& Value)
{
    return Value;
}

template<template<typename> class T>
std::unique_ptr<IDataBlockBase> CreateDataBlock(eFieldVariantType Type)
{
    switch (Type)
    {
    case eFieldVariantType::Monostate: return std::make_unique<T<FieldVariantData>>();
    case eFieldVariantType::Long: return std::make_unique<T<long>>();
    case eFieldVariantType::Double: return std::make_unique<T<double>>();
    case eFieldVariantType::Bool: return std::make_unique<T<bool>>();
    case eFieldVariantType::String: return std::make_unique<T<std::string>>();
    case eFieldVariantType::Uint64: return std::make_unique<T<uint64_t>>();
    default: return nullptr;
    }
}

const std::string_view MapFieldVariantName(eFieldVariantType Type)
{
    switch (Type)
    {
    case eFieldVariantType::Monostate: return MapFieldVariantType<FieldVariantData>::VerbalType();
    case eFieldVariantType::Long: return MapFieldVariantType<long>::VerbalType();
    case eFieldVariantType::Double: return MapFieldVariantType<double>::VerbalType();
    case eFieldVariantType::Bool: return MapFieldVariantType<bool>::VerbalType();
    case eFieldVariantType::String: return MapFieldVariantType<std::string>::VerbalType();
    case eFieldVariantType::Uint64: return MapFieldVariantType<uint64_t>::VerbalType();
    default: return nullptr;
    }
}


template<template<typename> class T>
std::unique_ptr<IDataBlockBase> CreateDataBlock(eFieldVariantType Type)
{
    switch (Type)
    {
    case eFieldVariantType::Monostate: return std::make_unique<T<FieldVariantData>>();
    case eFieldVariantType::Long: return std::make_unique<T<long>>();
    case eFieldVariantType::Double: return std::make_unique<T<double>>();
    case eFieldVariantType::Bool: return std::make_unique<T<bool>>();
    case eFieldVariantType::String: return std::make_unique<T<std::string>>();
    case eFieldVariantType::Uint64: return std::make_unique<T<uint64_t>>();
    default: return nullptr;
    }
}

template<typename T>
class DataBlockIndex : public T
{
protected:
    IndexesT Indexes_;
public:
    const IndexT* const Indexes() const noexcept override { return Indexes_.size() ? Indexes_.data() : nullptr; }
    IndexT* Indexes() noexcept override { return Indexes_.size() ? Indexes_.data() : nullptr; }
    IndexT IndexesSize() const noexcept override { return static_cast<IndexT>(Indexes_.size()); }
    IndexesT& IndexesVector() { return Indexes_; }
};

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

template<typename T>
class DataBlockSparse : public DataBlock<T>
{
public:
    T* Data() noexcept override { return nullptr; }
    const T* const Data() const noexcept override { return nullptr; }
    IndexT DataSize() const noexcept { return 0; };
    IPlainRastrRetCode Set(IndexT Row, IndexT Column, const T& Value) noexcept override
    {
        std::cout << "Sparse block set(" << Row << "," << Column << "," << VariantToString<T>::String(Value) << ")" << std::endl;
        return IPlainRastrRetCode::Ok;
    }
};

class DataSet : public DataBlockIndex<IDataSetBase>
{
    using BaseT = DataBlockIndex<IDataSetBase>;
protected:
    IndexT RowsCount_ = 0;
    struct ColumnHeader
    {
        std::unique_ptr<IDataBlockBase> Column;
        std::string Name;
    };
    std::vector<ColumnHeader> Columns_;
    // функция фабрика столбцов. Вы можете (и должны) будете наследовать свой класс столбца
    // от IDataBlockColumn, и с помощью этого вложенного шаблона укажете его в качестве
    // типа столбцов датасета
    template<template<typename> class T>
    std::unique_ptr<IDataBlockBase> CreateDataBlock(eFieldVariantType Type)
    {
        switch (Type)
        {
        case eFieldVariantType::Monostate: return std::make_unique<T<FieldVariantData>>();
        case eFieldVariantType::Long: return std::make_unique<T<long>>();
        case eFieldVariantType::Double: return std::make_unique<T<double>>();
        case eFieldVariantType::Bool: return std::make_unique<T<bool>>();
        case eFieldVariantType::String: return std::make_unique<T<std::string>>();
        case eFieldVariantType::Uint64: return std::make_unique<T<uint64_t>>();
        default: return nullptr;
        }
    }
public:
    IPlainRastrRetCode AddColumn(IndexT ColumnIndex, std::string_view Name, eFieldVariantType Type) noexcept override
    {
        try
        {
            // мы его создаем фабрикой, указывая тип __своего__ столбца, который делает то что вам нужно
            // я здесь использую тот же самый датаблок
            Columns_.emplace_back(ColumnHeader{ CreateDataBlock<DataBlock>(Type), std::string(Name) });
            // если что-то пошло не так с созданием - у нас есть код возврата
            if (Columns_.back().Column != nullptr)
                return IPlainRastrRetCode::Ok;
            Columns_.pop_back();
        }
        catch (const std::exception&)
        {
            return IPlainRastrRetCode::Failed;
        }
        return IPlainRastrRetCode::Failed;
    }

    IndexT RowsCount() const noexcept override { return RowsCount_; }
    void Clear()
    {
        Columns_.clear();
        RowsCount_ = 0;
    }
    IPlainRastrRetCode SetBlockSize(IndexT RowsCount, IndexT ColumnsCount, bool UseIndexes) noexcept override
    {
        try
        {
            BaseT::IndexesVector().clear();
            RowsCount_ = 0;
            if (ColumnsCount > static_cast<IndexT>(Columns_.size()))
                return IPlainRastrRetCode::Failed;

            for (const auto& Column : Columns_)

                if (Column.Column->SetBlockSize(RowsCount, 1) == IPlainRastrRetCode::Failed)

                    return IPlainRastrRetCode::Failed;
            BaseT::IndexesVector().resize(RowsCount);
            RowsCount_ = RowsCount;
        }
        catch (const std::exception&)
        {
            return IPlainRastrRetCode::Failed;
        }

        return IPlainRastrRetCode::Ok;
    }
    IndexT ColumnsCount() const noexcept override { return static_cast<IndexT>(Columns_.size()); }
    const IDataBlockBase* Column(IndexT ColumnIndex) const noexcept override
    {
        return ColumnIndex >= 0 && ColumnIndex < Columns_.size() ? Columns_[ColumnIndex].Column.get() : nullptr;
    }
    IDataBlockBase* Column(IndexT ColumnIndex) noexcept override
    {
        return ColumnIndex >= 0 && ColumnIndex < Columns_.size() ? Columns_[ColumnIndex].Column.get() : nullptr;
    }

    double Sparcity()
    {
        double Nzs{ 0 };
        for (const auto& column : Columns_)
            Nzs += column.Column->DataSize();
        return Nzs > 0.0 ? Nzs / (RowsCount() * ColumnsCount()) : Nzs;
    }

    void Dump()
    {
        std::cout << "DataSet " << RowsCount() << "x" << ColumnsCount() << std::endl;
        for (const auto& Column : Columns_)
            std::cout << Column.Name << "(" <<
                static_cast<std::underlying_type<eFieldVariantType>::type>(Column.Column->Type()) << ",[" <<
                Column.Column->DataSize() << "]" <<
                ");";
        std::cout << std::endl;
        for (IndexT row{ 0 }; row < RowsCount(); row++)
        {
            for (IndexT column = 0; column < ColumnsCount(); column++)
            {
                if (column)
                    std::cout << ";";
                else if (row < BaseT::IndexesVector().size())
                    std::cout << BaseT::IndexesVector()[row] << ";";

                const auto& col{ Column(column) };
                if (col->DataSize() == RowsCount())
                    std::cout << std::visit(ToString(), Column(column)->Get(row, 0));
                else
                {
                    auto CurrentIndex{ BaseT::IndexesVector()[row] };
                    auto first{ col->Indexes() };
                    auto last{ first + col->IndexesSize() };
                    auto locindex{ std::lower_bound(col->Indexes(), last, CurrentIndex) };
                    if(locindex != last && CurrentIndex == *locindex)
                        std::cout << std::visit(ToString(), Column(column)->Get(static_cast<IndexT>(locindex - first), 0));
                }
            }
            std::cout << std::endl;
        }
    }
};
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


#endif // RDATA_H
