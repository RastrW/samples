#ifndef QDATABLOCKS_H
#define QDATABLOCKS_H

#pragma once
#include <algorithm>
#include "IDataBlocksWrappers.h"

constexpr const char *cszDef = "[def]"; // "пустое" значение для разреженного режима

template<typename T>
class MyDenseDataBlock : public IDataBlockBaseT<T>
{
    // и реализуем виртуальные методы чтобы решить свою задачу
public:
    // Датаблок должен соообщать о размерностях,
    // поэтому мы вводим функции, возвращающие количество строк (записей)
    // и столбцов (полей).
    IndexT RowsCount() const noexcept override
    {
        // Для хранения размерности
        // внутри датаблока используем пару переменных
        // Дальше посмотрим как получаются их значения
        return RowsCount_;
    }
    IndexT ColumnsCount() const noexcept override
    {
        return static_cast<IndexT>(Columns_.size());
    }

    // Датаблок также должен сообщать свой тип, причем
    // в перечислении возможных типов FieldVariantData
    // из CPlain
    eFieldVariantType Type() const noexcept override
    {
        // тут можно использовать свою специализацию, но
        // я воспользуюсь уже готовой
        return MapFieldVariantType<T>::Type();
        // обратите внимание, что тип FieldVariantData, который
        // тоже может быть T датаблока мапится в std::monostate
        // это imperfection, но пока не стоит городить еще один enum
    }

    // Мы выбрали для начала плотный датаблок, поэтому он должен
    // предоставлять доступ к блоку памяти. Она может быть своей
    // или чужой, главное чтобы она была выделена на стороне клиента.
    // И для доступа к этой памяти на чтение и запись мы делаем два метода
    const T* const Data() const noexcept override
    {
        // в туториале мы выделяем память прямо в нашем
        // датаблоке. В другой реализации мы может
        // взять память еще где-то, например из объекта
        // того же питона
        return Data_.get();
    }
    T* Data() noexcept override
    {
        // как управлять этой памятью посмотрим дальше
        return Data_.get();
    }

    // Если мы работаем с блоком памяти, нам надо
    // знать какой у него размер
    IndexT DataSize() const noexcept override
    {
        // Если датаблок прямоугольный и плотный
        // достаточно просто сообщить его площадь
        return RowsCount() * ColumnsCount();
    }

    // Кроме памяти собственно под данные у датаблока
    // может быть еще память под индексы. Она всегда
    // непрерывная (без sparse), чтобы упростить жизнь
    // серверу, который их читает
    const IndexT* const Indexes() const noexcept override
    {
        return Indexes_.data();
    }
    // и записывает
    IndexT* Indexes() noexcept override
    {
        // Индексы проще всего хранить в векторе
        // Его размерность равна RowsCount()
        return Indexes_.data();
    }

    // Но для контроля корректности и способа
    // работы с данными есть отдельный метод
    // возвращающий размер индекса
    IndexT IndexesSize() const noexcept override
    {
        return static_cast<IndexT>(Indexes_.size());
    }

    // Когда сервер начинает передавать данные с в датаблок
    // он дает возможность клиенту определить все ли поля БД
    // будут передаваться и какие с точки зрения датаблока
    // у них типы. Смысл AddColumn в том, чтобы выровнять
    // индексы столбцов, которые должны одинаково себе представлять
    // клиент и сервер. Поскольку сервер может не передавать какие-то поля
    // клиенту предоставляется возможность получить список передаваемых полей.
    // С другой стороны клиент может отказаться принимать какое-то поле,
    // например из-за того, что не подошел тип. В этом случае он кодом
    // возврата информирует сервер что столбец не нужен и они оба, и
    // клиент и сервер это поле пропускают.

    IPlainRastrRetCode AddColumn(IndexT ColumnIndex, std::string_view Name, eFieldVariantType Type) noexcept override
    {
        // Мы могли бы просто дампить приходящие поля, но
        // сделаем чуть сложнее, и будет отказываться принимать
        // поля с типом, отличающегося от типа датаблока
        // Одно исключение сделаем для типа FieldVariandData - он может принимать любой тип
        // данных. Эту проверку правильно было бы сделать в специализации, но для наглядности/компактности
        // она в рантайме.
        if (this->Type() != eFieldVariantType::Monostate && Type != MapFieldVariantType<T>::Type())
            return IPlainRastrRetCode::Failed;
        // а те поля, которые на подходят соберем
        // в вектор и будем дампить
        Columns_.emplace_back(Name);
        return IPlainRastrRetCode::Ok;
    }

    // После того как сервер определил размерности передаваемого блока: подсчитал количество
    // столбцов (и утвердил их в AddColumn), а также количество строк - по выборке или по индексам
    // датаблока - он информирует датаблок о необходимой размерности блока памяти
    // Вообще RowsCount() логично равен размерности индексов. Но не всегда. Поэтому мы для
    // строк используем отдельную переменную, а не Indexes_.size(). Отказ от индексов
    // может быть инициирован сервером, и для этого в параметрах есть UseIndexes. Эта фича
    // используется при работе с датасетом. Здесь мы ее использовать не будем. Тем не менее,
    // если мы будем возвращать серверу нулевые индексы (и размер и указатель), он будет использовать
    // датаблок по индексам выборки в пределах его размерности. Это странный маппинг, но единственно
    // возможный в такой ситуации.
    IPlainRastrRetCode SetBlockSize(IndexT RowSize, IndexT ColumnSize, bool UseIndexes) noexcept override
    {
        // Мы набирали столбцы в AddColumn. Проверим, правильно ли
        // мы с сервером их посчитали ?
        if (static_cast<IndexT>(Columns_.size()) != ColumnSize)
            return IPlainRastrRetCode::Failed;

        // Все что может привести к исключению
        // нужно стараться прятать в try/catch.
        // Если с выделением памяти в разных хипах
        // еще как-то можно жить, то исключение точно
        // приведет к "трапу"
        // trap to 4, как говорила pdp-11 :)
        try
        {
            // если всё как договорились с сервером
            // выделяем память и фиксируем размерности
            Indexes_.clear();
            RowsCount_ = RowSize;
            // если сервер говорит что индексы не нужны
            // (он не собирается "сжимать" столбцы)
            // то обнуляем их и не выделяем новые
            if(UseIndexes)
                Indexes_.resize(RowSize);
            Data_ = std::make_unique<T[]>(RowsCount() * ColumnsCount());
            return IPlainRastrRetCode::Ok;
        }
        catch (const std::exception&)
        {
            // если у нас не получилось,
            // важно сделать так чтобы сервер
            // это понимал не только по коду возврата
            Clear();
            // для удобства сделаем функцию
            // очистки памяти датаблока, она
            // нам пригодится и в тестах
            return IPlainRastrRetCode::Failed;
        }
    }

    // Этот метод предназначен для разреженной передачи на сервер.
    // Мы пока ее не будем использовать
    IPlainRastrRetCode Get(IDataBlockSetter<T>& Setter) const noexcept override
    {
        return IPlainRastrRetCode::NotImplemented;
    }

    // Здесь, как многократно мы рассуждали копирование в память клиента
    // просто вынесено в отдельную функцию. Она используется сервером для типов,
    // требующих выделения памяти в хипе клиента
    void Set(T* ptr, const T& Value) override
    {
        *ptr = Value;
    }

    // Этот метод предназначен для разреженной передачи клиенту.
    // Мы уже очень скоро её попробуем
    IPlainRastrRetCode Set(IndexT RowIndex, IndexT ColumnIndex, const T& Value) noexcept override
    {
        return IPlainRastrRetCode::NotImplemented;
    }

    // Вот и все абстрактные методы, которые нужно реализовать
    // для того чтобы заработал плотный датаблок с монотипом


    // Остались наши удобняшки для теста
    void Clear()
    {
        RowsCount_ = 0;
        Columns_.clear();
        Data_.reset();
        Indexes_.clear();
    }

    // И простейший дампер
    void Dump(long LimitRows = (std::numeric_limits<long>::max)())
    {
        // Нам в дампере пригодится возможность показывать тип
        std::cout << "DataBlock dense [" << MapFieldVariantType<T>::VerbalType() << "] " << RowsCount() << "x" << ColumnsCount() << std::endl;
        if (Indexes_.size())
            std::cout << "#;";

        // А также то, что мы собрали имена полей
        for (const auto& Column : Columns_)
            std::cout << Column << ";";

        std::cout << std::endl;

        // Функции преобразования в строки я использую готовые
        for (IndexT row = 0; row < RowsCount() && row < LimitRows; row++)
        {
            for (IndexT column = 0; column < ColumnsCount(); column++)
            {
                if (column) std::cout << ";";
                else if (row < static_cast<IndexT>(Indexes_.size()))
                    std::cout << Indexes_[row] << ";";
                std::cout << VariantToString<T>::String(Data()[row * ColumnsCount() + column]);
            }
            std::cout << std::endl;
        }
    }

    // Для теста записи - реверс индексов
    void Reverse()
    {
        std::reverse(Indexes_.begin(), Indexes_.end());
    }

    std::string Columns() const
    {
        return stringutils::join(Columns_,',');
    }

    // функция получения значения по индексу строки и столбца в виде варианта
    // Она нам нужна для дамперов. В обмене с севером не участвует.
    FieldVariantData Get(IndexT RowIndex, IndexT ColumnIndex)  const override
    {
        if (Indexes_.size())
        {
            if (auto locindex{ std::lower_bound(Indexes_.begin(), Indexes_.end(), RowIndex) };
                locindex != Indexes_.end() && *locindex == RowIndex)
                return Data()[(locindex - Indexes_.begin()) * ColumnsCount() + ColumnIndex];
            else
                return {};
        }
        else
        {
            if (RowIndex >= 0 && RowIndex < RowsCount() &&
                ColumnIndex >= 0 && ColumnIndex < ColumnsCount())
                return Data()[RowIndex * ColumnsCount() + ColumnIndex];
            else
                return {};
        }
    }

    // Метод, который каким-то образом изменяет данные
    // в хранилище, чтобы было видно что мы отдали
    // на сервер новые данные

    static T Shuffle(const T& Value);

    void Shuffle()
    {
        for (T* p{ Data() }; p < Data() + DataSize(); ++p)
            *p = Shuffle(*p);
    }

protected:
    IndexT RowsCount_ = 0;
    std::unique_ptr<T[]> Data_ = nullptr;
    IndexesT Indexes_;
    std::vector<std::string> Columns_;
};

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

template<typename T>
class QDenseDataBlock : public MyDenseDataBlock<T>
{
    public :
    IPlainRastrRetCode Set(IndexT RowIndex, IndexT ColumnIndex, const T& Value) noexcept override
    {
        try {
            this->Data()[RowIndex * this->ColumnsCount() + ColumnIndex] = Value;
            return IPlainRastrRetCode::Ok;
        } catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }
    IPlainRastrRetCode AddRow(IndexT count = 1) noexcept
    {
        try {

            IndexT NewRowsCount_ = this->RowsCount() + count;
            std::unique_ptr<T[]> NewData_ = std::make_unique<T[]>(NewRowsCount_ * this->ColumnsCount());
            for (int row = 0 ; row < this->RowsCount() ; row++)
            {
                for (int col = 0 ; col < this->ColumnsCount() ; col++)
                {
                    size_t indx = row * this->ColumnsCount() + col;
                    NewData_[indx]  = this->Data()[indx];
                }
            }

            //size_t sz_0 = sizeof(this->Data()[0]);
            //std::memcpy(NewData_.get(),this->Data(),NewRowsCount_ * this->ColumnsCount() * 4);
            //std::memmove(NewData_.get(),this->Data(),)

            this->RowsCount_ = NewRowsCount_;
            this->Data_ = std::move( NewData_);

            return IPlainRastrRetCode::Ok;
        } catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }
    IPlainRastrRetCode InsertRow(IndexT insrow ) noexcept
    {
        try {

            IndexT NewRowsCount_ = this->RowsCount() + 1;
            std::unique_ptr<T[]> NewData_ = std::make_unique<T[]>(NewRowsCount_ * this->ColumnsCount());
            for (int row = 0 ; row < insrow ; row++)
            {
                for (int col = 0 ; col < this->ColumnsCount() ; col++)
                {
                    size_t indx = row * this->ColumnsCount() + col;
                    NewData_[indx]  = this->Data()[indx];
                }
            }

            for (int row = insrow + 1 ; row < NewRowsCount_ ; row++)
            {
                for (int col = 0 ; col < this->ColumnsCount() ; col++)
                {
                    size_t indx_old = (row - 1) * this->ColumnsCount() + col;
                    size_t indx_new = row * this->ColumnsCount() + col;
                    NewData_[indx_new]  = this->Data()[indx_old];
                }
            }

            this->RowsCount_ = NewRowsCount_;
            this->Data_ = std::move( NewData_);

            return IPlainRastrRetCode::Ok;
        } catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }
};

//template<typename T>
class QDataBlock : public QDenseDataBlock<FieldVariantData>
{

};


#endif // RDATABLOCKS_H
