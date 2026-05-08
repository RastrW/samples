#pragma once

#include "iostream"

using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>
//#include "IPlainRastrWrappers.h"
#include <spdlog/spdlog.h>

// и вот тут мы наследуемся от базовых классов датаблоков, чтобы решать свои задачи

struct ToString {
    std::string operator()(std::monostate) { return { "" }; }
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
            spdlog::debug(str_row_vals.c_str());
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
