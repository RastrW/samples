#ifndef IDATABLOCKSWRAPPERS_H
#define IDATABLOCKSWRAPPERS_H
#pragma once

#include "iostream"
#include <QDebug>

using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>
//#include "IPlainRastrWrappers.h"


// и вот тут мы наследуемся от базовых классов датаблоков, чтобы решать свои задачи

struct ToString {
    std::string operator()(std::monostate) { return { "def" }; }
    std::string operator()(const long& value) { return std::to_string(value); }
    std::string operator()(const uint64_t& value) { return std::to_string(value); }
    std::string operator()(const double& value) { return std::to_string(value); }
    std::string operator()(const bool& value) { return value ? "1" : "0"; }
    std::string operator()(const std::string& value) { return value; }
};

struct ToDouble {
    double operator()(std::monostate) { return  0.0; }
    double operator()(const long& value) { return (double)value; }
    double operator()(const uint64_t& value) { return value; }
    double operator()(const double& value) { return value; }
    double operator()(const bool& value) { return value; }
    double operator()(const std::string& value) { return std::stod(value); }
};
struct ToLong {
    long operator()(std::monostate) { return  0.0; }
    long operator()(const long& value) { return (long)value; }
    long operator()(const uint64_t& value) { return value; }
    long operator()(const double& value) { return value; }
    long operator()(const bool& value) { return value; }
    long operator()(const std::string& value) { return std::stol(value); }
};

template<typename T>
struct MapFieldVariantType
{
    static eFieldVariantType Type() noexcept;
    static const std::string_view VerbalType() noexcept;
};






template<typename T>
struct VariantToString
{
    static std::string String(const T& Value)
    {
        return std::to_string(Value);
    }
};



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

// датаблок с монотипом и собственным хранилищем
// которое можно отдавать в петон или куда подальше

template<typename T>
class DataBlock : public DataBlockIndex<IDataBlockBaseT<T>>
{
    using BaseT = DataBlockIndex<IDataBlockBaseT<T>>;
protected:
    IndexT RowsCount_ = 0;
    IndexT ColumnsCount_ = 0;
    T* data_ = nullptr;
public:
    ~DataBlock()
    {
        delete[] data_;
    }

    void Clear()
    {
        delete[] data_;
        data_ = nullptr;
        RowsCount_ = ColumnsCount_ = 0;
        BaseT::IndexesVector().clear();
    }

    IndexT RowsCount() const noexcept override { return RowsCount_; }
    IndexT ColumnsCount() const noexcept override { return ColumnsCount_; }
    eFieldVariantType Type() const noexcept override { return MapFieldVariantType<T>::Type(); }
    const T* const Data() const noexcept override { return data_; }
    T* Data() noexcept override { return data_; }
    IndexT DataSize() const noexcept override { return ColumnsCount() * RowsCount(); };
    IPlainRastrRetCode SetBlockSize(IndexT RowsCount, IndexT ColumnsCount, bool UseIndexes) noexcept override
    {
        try
        {
            Clear();
            if(UseIndexes)
                BaseT::IndexesVector().resize(RowsCount);

            data_ = new T[RowsCount * ColumnsCount]();
            RowsCount_ = RowsCount;
            ColumnsCount_ = ColumnsCount;
        }
        catch (const std::exception&)
        {
            return IPlainRastrRetCode::Failed;
        }
        return IPlainRastrRetCode::Ok;
    }

    // эту функцию мы можем использовать для датаблоков, у которых нет памяти
    // ва реализации ваш датаблок может полученное значение куда-то отдавать. Ну скажем в грид
    IPlainRastrRetCode Set(IndexT Row, IndexT Column, const T& Value) noexcept override
    {
        return IPlainRastrRetCode::NotImplemented;
    }

    // хелпер для аллокации в клиенте для нефундаментальных типов
    void Set(T* ptr, const T& Value) override
    {
        *ptr = Value;
    }

    // функция добавления столбца это то что раньше было Map. Здесь мы получаем все то же самое + тип
    // можем проверить вообще тип из растра подходит для вашего монотипа или как
    IPlainRastrRetCode AddColumn(IndexT ColumnIndex, std::string_view Name, eFieldVariantType Type) noexcept override
    {
        return IPlainRastrRetCode::Ok;
    }

    // ну дампер
    void Dump(long LimitRows = (std::numeric_limits<long>::max)())
    {
       /* for (IndexT row = 0; row < RowsCount() && row < LimitRows; row++)
        {
            for (IndexT column = 0; column < ColumnsCount(); column++)
            {
                if (column)
                    std::cout << ";";
                else if (row < BaseT::IndexesVector().size())
                    std::cout << BaseT::IndexesVector()[row] << ";";
                std::cout << VariantToString<T>::String(Data()[row * ColumnsCount() + column]);
            }
            std::cout << std::endl;
        }*/

    }
    void QDump(long LimitRows = (std::numeric_limits<long>::max)() , long LimitCols = (std::numeric_limits<long>::max)())
    {
        std::string str_row_vals = "";
        for (IndexT row = 0; row < RowsCount() && row < LimitRows; row++)
        {
            for (IndexT column = 0; column < ColumnsCount() && column < LimitCols; column++)
            {
                if (column)
                    str_row_vals.append( ";");
                else if (row < BaseT::IndexesVector().size())
                {
                    //str_row_vals.append(BaseT::IndexesVector()[row]);
                    str_row_vals.append(";");
                }
               str_row_vals.append(std::visit(ToString(),Data()[row * ColumnsCount() + column]));
            }
            qDebug() <<str_row_vals.c_str();
        }
    }

    IPlainRastrRetCode Get(IDataBlockSetter<T>& Setter) const noexcept override
    {
        for (IndexT RowIndex{ 0 }; RowIndex < RowsCount(); RowIndex++)
            for (IndexT ColumnIndex{ 0 }; ColumnIndex < ColumnsCount(); ColumnIndex++)
                Setter.Set(RowIndex, ColumnIndex, {});
        return IPlainRastrRetCode::Ok;
    }

    FieldVariantData Get(IndexT RowIndex, IndexT ColumnIndex)
    {
        auto pData{ Data() };
        if (DataSize() == RowsCount())
            return pData[RowIndex * ColumnsCount() + ColumnIndex];
        else
            return {};
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
        //std::cout << "Sparse block set(" << Row << "," << Column << "," << VariantToString<T>::String(Value) << ")" << std::endl;
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

            /*for (const auto& Column : Columns_)
                if (Column.Column->SetBlockSize(RowsCount, 1) == IPlainRastrRetCode::Failed)
                    return IPlainRastrRetCode::Failed;*/
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
      /*  std::cout << "DataSet " << RowsCount() << "x" << ColumnsCount() << std::endl;
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
      */
    }
};

#endif // IDATABLOCKSWRAPPERS_H
