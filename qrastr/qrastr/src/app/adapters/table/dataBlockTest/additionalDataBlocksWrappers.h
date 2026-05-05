#pragma once
#include "../IDataBlocksWrappers.h"

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

    void Dump(){}
};