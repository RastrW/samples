#pragma once
#include "../QDataBlocks.h"
#include "additionalDataBlocksWrappers.h"

constexpr const char *cszDef = "[def]"; // "пустое" значение для разреженного режима

// Переходим к разреженному блоку. Он почти такой же как и плотный, и
// можно было бы его просто наследовать и игнорировать ненужные члены, но
// мы сделаем с нуля
template<typename T>
class MySparseDataBlock : public IDataBlockBaseT<T>
{
public:
    IndexT RowsCount() const noexcept override
    {
        return RowsCount_;
    }

    IndexT ColumnsCount() const noexcept override
    {
        return static_cast<IndexT>(Columns_.size());
    }

    eFieldVariantType Type() const noexcept override
    {
        return MapFieldVariantType<T>::Type();
    }

    // У разреженного блока нет необходимости в блоке памяти
    const T* const Data() const noexcept override
    {
        return nullptr;
    }

    T* Data() noexcept override
    {
        return nullptr;
    }

    IndexT DataSize() const noexcept override
    {
        return 0;
    }

    // Индексы ему тоже не нужны
    const IndexT* const Indexes() const noexcept override
    {
        return nullptr;
    }

    IndexT* Indexes() noexcept override
    {
        return nullptr;
    }

    IndexT IndexesSize() const noexcept override
    {
        return 0;
    }

    // контроль и подсчет столбцов такой же как и у плотного блока
    IPlainRastrRetCode AddColumn(IndexT ColumnIndex, std::string_view Name, eFieldVariantType Type) noexcept override
    {
        if (this->Type() != eFieldVariantType::Monostate && Type != MapFieldVariantType<T>::Type())
            return IPlainRastrRetCode::Failed;
        Columns_.emplace_back(ColumnHeaderT{ Name, ColumnIndex });
        return IPlainRastrRetCode::Ok;
    }

    // При обработке размерности не выделяем память
    IPlainRastrRetCode SetBlockSize(IndexT RowSize, IndexT ColumnSize, bool UseIndexes) noexcept override
    {
        // Мы набирали столбцы в AddColumn. Проверим, правильно ли
        // мы с сервером их посчитали ?
        RowsCount_ = 0;
        if (static_cast<IndexT>(Columns_.size()) != ColumnSize)
            return IPlainRastrRetCode::Failed;
        RowsCount_ = RowSize;
        return IPlainRastrRetCode::Ok;
    }

    // А вот эттот метод нам теперь нужна. Она позволяет отдать на сервер
    // значения по индексам строк и столбцов
    IPlainRastrRetCode Get(IDataBlockSetter<T>& Setter) const noexcept override
    {
        // сервер при вызове метода передает _свой_ интерфейс для передачи значений
        // с индексами строк и столбцов. В нашем примере есть хранилище, и мы
        // будем передавать серверу значения из него
        IPlainRastrRetCode SetterRetCode = IPlainRastrRetCode::Ok;
        for (const auto& Column : Columns_)
        {
            if (SetterRetCode != IPlainRastrRetCode::Ok)
                break;
            for (const auto& Row : Column.Rows_)
                if (SetterRetCode = Setter.Set(Row.first, Column.Index_, Row.second);
                    SetterRetCode != IPlainRastrRetCode::Ok)
                    break;
        }
        return SetterRetCode;
    }


    // Этот метод сервер будет вызывать для того чтобы передать значение клиенту
    IPlainRastrRetCode Set(IndexT RowIndex, IndexT ColumnIndex, const T& Value) noexcept override
    {
        // В примере просто ставим значение по координатам в хранилище
        // Вместо хранилища может быть и пустой грид, в котором мы заполним
        // сколько-то значений
        Columns_[ColumnIndex].Rows_[RowIndex] = Value;
        return IPlainRastrRetCode::Ok;
    }

    // поскольку выделенной памяти нет, этот сеттер не имеет смысла
    void Set(T* ptr, const T& Value) override { }

    void Clear()
    {
        RowsCount_ = 0;
        Columns_.clear();
    }

    // Дампер
    void Dump(long LimitRows = (std::numeric_limits<long>::max)())
    {
        // собираем индексы строк из хранилища
        std::set<IndexT> RowIndexes;
        IndexT NzValues{ 0 };
        for (const auto& Column : Columns_)
            for (const auto& Row : Column.Rows_)
            {
                RowIndexes.emplace(Row.first);
                ++NzValues; // считаем количество ненулевых элементов для иллюстрации преимуществ
            }

        const double Square{ static_cast<double>(RowsCount()) * ColumnsCount() };
        std::cout << "DataBlock sparse [" << MapFieldVariantType<T>::VerbalType() << "] " << RowsCount() << "x" << ColumnsCount() <<
            "sparsity " << (Square ? 100 * (Square - NzValues) / Square : 100.0) << "%" << std::endl;

        for (const auto& Column : Columns_)
            std::cout << Column.Name_ << ";";
        std::cout << std::endl;


        for (const auto& Row : RowIndexes)
        {
            std::cout << Row << ";";
            bool RowStart{ true };
            for (const auto& Column : Columns_)
            {
                if (RowStart)
                    RowStart = false;
                else
                    std::cout << ";";

                if (auto rowdata{ Column.Rows_.find(Row) }; rowdata != Column.Rows_.end())
                    std::cout << VariantToString<T>::String(rowdata->second);
                else
                    std::cout << cszDef;
            }
            std::cout << std::endl;
        }
    }

    std::string Columns() const
    {
        std::string ColumnList;
        for (const auto& Column : Columns_)
        {
            if (!ColumnList.empty())
                ColumnList.push_back(';');
            ColumnList.append(Column.Name_);
        }
        return ColumnList;
    }

    void Shuffle()
    {
        for (auto&& Column : Columns_)
            for (auto&& Row : Column.Rows_)
                Row.second = MyDenseDataBlock<T>::Shuffle(Row.second);
    }

protected:
    IndexT RowsCount_ = 0;
    // наше тестовое хранилище будет по столбцам
    struct ColumnHeaderT
    {
        std::string Name_;
        IndexT Index_;
        std::map<IndexT, T> Rows_;

        ColumnHeaderT(std::string_view Name, IndexT Index) : Name_(Name), Index_(Index) {}
    };
    std::vector<ColumnHeaderT> Columns_;
};

// Датасет представляет собой коллекцию датаблоков, с монотипами.
// Интерфейс датасета наследован он IDataBlockBase, то есть он
// представляется в виде датаблока со столбцами с произвольными типами.
// К методам датаблока добавляются два метода для работы со столбцами

class MyDataSet : public IDataSetBase
{
public:
    // Датасет сам должен управлять столбцами,
    // поэтому вводим структуру, в которой будем
    // хранить сами столбцы и метаданные
    struct ColumnHeaderT
    {
        // столбцом является уже знакомый нам датаблок
        std::unique_ptr<IDataBlockBase> Column_;
        std::string Name_;
    };

    // Датасет имеет собственные индексы.
    // По ним разумно устанавливать количество строк в датасете
    IndexT RowsCount() const noexcept override
    {
        return static_cast<IndexT>(Indexes_.size());
    }

    // Количество столбцов датасет знает из коллекции столбцов
    IndexT ColumnsCount() const noexcept override
    {
        return static_cast<IndexT>(Columns_.size());
    }

    // Доступ к индексам такой же как в датаблоке
    const IndexT* const Indexes() const noexcept override
    {
        return Indexes_.data();
    }

    IndexT* Indexes() noexcept override
    {
        return Indexes_.data();
    }

    IndexT IndexesSize() const noexcept override
    {
        return RowsCount();
    }

    // При добавлении столбца датасет должен его создать, в отличие от датаблока
    IPlainRastrRetCode AddColumn(IndexT ColumnIndex, std::string_view Name, eFieldVariantType Type) noexcept override
    {
        try
        {
            // Мы его создаем фабрикой, указывая тип __своего__ датаблока, который делает то что вам нужно
            // В примере мы используем наш уже готовый плотный датаблок
            Columns_.emplace_back(ColumnHeaderT{ CreateDataBlock<MyDenseDataBlock>(Type), std::string(Name) });
            if (Columns_.back().Column_ != nullptr)
                return Columns_.back().Column_->AddColumn(0, Name, Type);
            Columns_.pop_back();
            return IPlainRastrRetCode::Ok;
        }
        catch (const std::exception&)
        {
            return IPlainRastrRetCode::Failed;
        }
    }

    // При обработке размерности не выделяем память
    IPlainRastrRetCode SetBlockSize(IndexT RowSize, IndexT ColumnSize, bool UseIndexes) noexcept override
    {
        // Но очищаем индексы
        try
        {
            Indexes_.clear();
            if (static_cast<IndexT>(Columns_.size()) != ColumnSize)
                return IPlainRastrRetCode::Failed;
            // И если все получается, резервируем размерность индексов по заданному количеству строк
            Indexes_.resize(RowSize);
            return IPlainRastrRetCode::Ok;
        }
        catch (const std::exception&)
        {
            return IPlainRastrRetCode::Failed;
        }
    }

    // Два дополнительных метода датасета - возврат столбцов по индексу, в const/volatile версиях
    const IDataBlockBase* Column(IndexT ColumnIndex) const noexcept override
    {
        return ColumnIndex >= 0 && ColumnIndex < ColumnsCount() ? Columns_[ColumnIndex].Column_.get() : nullptr;
    }

    IDataBlockBase* Column(IndexT ColumnIndex) noexcept override
    {
        return ColumnIndex >= 0 && ColumnIndex < ColumnsCount() ? Columns_[ColumnIndex].Column_.get() : nullptr;
    }

    // Свойство датасета, которое позволяет серверу включить
    // режим разреженной передачи данных - когда столбец заполняется
    // _по_индексам_ столбца только недефолтными значениями.
    bool AllowSparse() const noexcept override { return true; }

    void Dump();

    template<typename T>
    void Set(IndexT ColumnIndex , IndexT RowIndex ,const T& Value )
    {
        IDataBlockBase* Col = Column(ColumnIndex);


        switch (Col->Type())
        {
        case eFieldVariantType::Monostate:
            // static_cast<MyDenseDataBlock<FieldVariantData>*>(Col)->Set(0,RowIndex,Value);
            break;
        case eFieldVariantType::Long:
            // static_cast<MyDenseDataBlock<long>*>(Col)->Set(0,RowIndex,(const long)Value);
            break;
        case eFieldVariantType::Double:
            // static_cast<MyDenseDataBlock<double>*>(Col)->Set(0,RowIndex,(double)Value);
            break;
        case eFieldVariantType::String:
            // static_cast<MyDenseDataBlock<std::string>*>(Col)->Set(0,RowIndex,(std::string)Value);
            break;
        case eFieldVariantType::Bool:
            //static_cast<MyDenseDataBlock<bool>*>(Col)->Set(0,RowIndex,(bool)Value);
            break;
        }
    }


    void Shuffle()
    {
        for (auto&& Column : Columns_)
        {
            auto& Col{ Column.Column_ };
            switch (Col->Type())
            {
            case eFieldVariantType::Monostate:
                static_cast<MyDenseDataBlock<FieldVariantData>*>(Col.get())->Shuffle();
                break;
            case eFieldVariantType::Long:
                static_cast<MyDenseDataBlock<long>*>(Col.get())->Shuffle();
                break;
            case eFieldVariantType::Double:
                static_cast<MyDenseDataBlock<double>*>(Col.get())->Shuffle();
                break;
            case eFieldVariantType::String:
                static_cast<MyDenseDataBlock<std::string>*>(Col.get())->Shuffle();
                break;
            case eFieldVariantType::Bool:
                static_cast<MyDenseDataBlock<bool>*>(Col.get())->Shuffle();
                break;
            }
        }
    }

    void Clear()
    {
        Columns_.clear();
        Indexes_.clear();
    }

    std::string Columns() const
    {
        std::string ColumnList;
        for (const auto& Column : Columns_)
        {
            if (!ColumnList.empty())
                ColumnList.push_back(';');
            ColumnList.append(Column.Name_);
        }
        return ColumnList;
    }
protected:
    std::vector<IndexT> Indexes_;
    std::vector<ColumnHeaderT> Columns_;
};

