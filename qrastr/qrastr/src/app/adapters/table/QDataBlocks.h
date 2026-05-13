#pragma once
#include <algorithm>
#include "IDataBlocksWrappers.h"

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

///
template<typename T>
class QDenseDataBlock : public MyDenseDataBlock<T>
{
public :
    void QDump(long LimitRows = (std::numeric_limits<long>::max)() , long LimitCols = (std::numeric_limits<long>::max)())
    {
        // Нам в дампере пригодится возможность показывать тип
        spdlog::debug("DataBlock dense [{},{}]", this->RowsCount(), this->ColumnsCount());
        if (this->Indexes_.size())
            spdlog::debug("#;");
        spdlog::debug("LimitRows = {}", LimitRows);
        spdlog::debug("LimitCols = {}", LimitCols);

        // А также то, что мы собрали имена полей
        std::string str_columns_all = stringutils::join(this->Columns_,';');        // все столбцы
        std::string str_columns = "";
        for (IndexT column = 0; column < this->ColumnsCount() && column < LimitCols; column++)
        {
            str_columns.append(this->Columns_[column]);
            str_columns.append(";");
        }

        spdlog::debug(str_columns);

        // Функции преобразования в строки я использую готовые
        for (IndexT row = 0; row < this->RowsCount() && row < LimitRows; row++)
        {
            std::string str_row_vals = "";
            for (IndexT column = 0; column < this->ColumnsCount() && column < LimitCols; column++)
            {
                if (column) str_row_vals.append(";");
                else if (row < static_cast<IndexT>(this->Indexes_.size()))
                    spdlog::debug("row_index: {}", std::to_string(this->Indexes_[row]));

                str_row_vals.append(std::visit(ToString(),this->Data()[row * this->ColumnsCount() + column]));
            }
            spdlog::debug(str_row_vals);
        }
    }

    IPlainRastrRetCode Set(IndexT RowIndex, IndexT ColumnIndex, const T& Value) noexcept override
    {
        try {
            this->Data()[RowIndex * this->ColumnsCount() + ColumnIndex] = Value;
            return IPlainRastrRetCode::Ok;
        } catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }
    IPlainRastrRetCode Set(IndexT RowIndex, std::string_view ColumnName, const T& Value) noexcept
    {
        IndexT ColumnIndex = -1;
        for (int i = 0 ; i < this->ColumnsCount() ; i++ )
            if (this->Columns_[i] == ColumnName)
            {
                ColumnIndex = i;
                break;
            }
        if (ColumnIndex < 0)
            return IPlainRastrRetCode::Failed;

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
            const IndexT newRows = this->RowsCount_ + count;
            const IndexT cols    = static_cast<IndexT>(this->Columns_.size());
            auto newData = std::make_unique<T[]>(
                static_cast<size_t>(newRows) * cols);

            copyRows(newData.get(), this->Data_.get(),
                     0, 0, this->RowsCount_, cols);   // копируем старые строки

            // новые строки value-initialized (нули/пустые варианты) — уже сделано make_unique

            for (IndexT i = 0; i < count; ++i)
                this->Indexes_.push_back(this->RowsCount_ + i);

            this->RowsCount_ = newRows;
            this->Data_      = std::move(newData);
            return IPlainRastrRetCode::Ok;
        } catch (...) { return IPlainRastrRetCode::Failed; }
    }
    IPlainRastrRetCode InsertRow(IndexT insrow) noexcept
    {
        try {
            const IndexT oldRows = this->RowsCount_;
            const IndexT cols    = static_cast<IndexT>(this->Columns_.size());
            auto newData = std::make_unique<T[]>(
                static_cast<size_t>(oldRows + 1) * cols);

            copyRows(newData.get(), this->Data_.get(),
                     0, 0, insrow, cols);              // строки до insrow

            // строка insrow — нулевая (value-init)

            copyRows(newData.get(), this->Data_.get(),
                     insrow + 1, insrow, oldRows - insrow, cols); // строки после
            //Нужно добавить индекс для еще одной строки
            this->Indexes_.push_back(oldRows);
            this->RowsCount_ = oldRows + 1;
            this->Data_      = std::move(newData);
            return IPlainRastrRetCode::Ok;
        } catch (...) { return IPlainRastrRetCode::Failed; }
    }
    IPlainRastrRetCode DuplicateRow(IndexT duprow) noexcept
    {
        // К этому моменту handleInsertRow уже вставил пустую строку сверху от исходной.
        // Копируем данные из оригинала (duprow+1) в новую строку (duprow).
        try {
            for (int col = 0; col < this->ColumnsCount(); ++col) {
                size_t dst = duprow       * this->ColumnsCount() + col;
                size_t src = (duprow + 1) * this->ColumnsCount() + col;
                this->Data()[dst] = this->Data()[src];
            }
            return IPlainRastrRetCode::Ok;
        } catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }
    IPlainRastrRetCode DeleteRow(IndexT delrow) noexcept
    {
        try {
            const IndexT oldRows = this->RowsCount_;
            const IndexT cols    = static_cast<IndexT>(this->Columns_.size());
            auto newData = std::make_unique<T[]>(
                static_cast<size_t>(oldRows - 1) * cols);

            copyRows(newData.get(), this->Data_.get(),
                     0, 0, delrow, cols);                          // строки до
            copyRows(newData.get(), this->Data_.get(),
                     delrow, delrow + 1, oldRows - delrow - 1, cols); // строки после
            //Нужно удалить индекс последней строки
            this->Indexes_.pop_back();
            this->RowsCount_ = oldRows - 1;
            this->Data_      = std::move(newData);
            return IPlainRastrRetCode::Ok;
        } catch (...) { return IPlainRastrRetCode::Failed; }
    }

    /// Копирует строки [srcFrom, srcFrom+count) из src в dst начиная с dstFrom.
    static void copyRows(T* dst, const T* src,
                         IndexT dstFrom, IndexT srcFrom, IndexT count,
                         IndexT cols) noexcept
    {
        std::copy(src  + srcFrom * cols,
                  src  + (srcFrom + count) * cols,
                  dst  + dstFrom * cols);
    }
    /// Дозагружает одну колонку из одноколоночного блока-источника.
    /// Если колонка уже есть — ничего не делает.
    IPlainRastrRetCode mergeColumn(const std::string& name,
                                   const QDenseDataBlock<T>& src) noexcept
    {
        if (localColumnIndex(name) >= 0)
            return IPlainRastrRetCode::Ok; // уже есть

        if (src.RowsCount() != this->RowsCount_ && this->RowsCount_ > 0)
            return IPlainRastrRetCode::Failed; // несовместимые размеры

        try {
            const IndexT oldCols = static_cast<IndexT>(this->Columns_.size());
            const IndexT newCols = oldCols + 1;

            auto newData = std::make_unique<T[]>(
                static_cast<size_t>(this->RowsCount_) * newCols);

            for (IndexT row = 0; row < this->RowsCount_; ++row) {
                // Копируем старые колонки
                for (IndexT col = 0; col < oldCols; ++col)
                    newData[row * newCols + col] =
                        std::move(this->Data_[row * oldCols + col]);
                // Дописываем новую (src — одноколоночный блок)
                newData[row * newCols + oldCols] = src.Data()[row];
            }

            this->Data_   = std::move(newData);
            this->Columns_.push_back(name);
            return IPlainRastrRetCode::Ok;
        }
        catch (...) {
            return IPlainRastrRetCode::Failed;
        }
    }

    /// Возвращает локальный индекс колонки в блоке по имени, -1 если нет
    long localColumnIndex(const std::string& name) const noexcept
    {
        for (IndexT i = 0; i < static_cast<IndexT>(this->Columns_.size()); ++i)
            if (this->Columns_[i] == name)
                return static_cast<long>(i);
        return -1;
    }
    std::string columnName(IndexT li) const noexcept {
        if (li < 0 || static_cast<size_t>(li) >= this->Columns_.size())
            return {};
        return this->Columns_[li];
    }
};

//template<typename T>
class QDataBlock : public QDenseDataBlock<FieldVariantData>
{

};
