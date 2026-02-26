#include "rtablesdatamanager.h"

void RTablesDataManager::setQAstra( QAstra* _pqastra)
{
    m_pqastra = _pqastra;
    connect(m_pqastra, &QAstra::onRastrHint, this, &RTablesDataManager::onRastrHint);
}

void  RTablesDataManager::setForms ( std::list<CUIForm>* _lstUIForms)
{
    m_plstUIForms = _lstUIForms;
}

CUIForm*  RTablesDataManager::getForm ( std::string _name)
{
    for (CUIForm &form : *m_plstUIForms)
    {
        if (stringutils::MkToUtf8(form.Name()) == _name)
            return &form;
    }
    return nullptr;
}

void  RTablesDataManager::onRastrHint(const _hint_data& hint_data)
{
    long row = hint_data.n_indx;
    std::string cname = hint_data.str_column;
    std::string tname = hint_data.str_table;
    qInfo() << "RTDM hint:"
             << static_cast<int>(hint_data.hint)
             << "table:" << tname.c_str()
             << "col:"   << cname.c_str()
             << "row:"   << row;

    switch (hint_data.hint)
    {
    case EventHints::ChangeAll:    handleChangeAll();                    break;
    case EventHints::ChangeTable:  handleChangeTable(tname);             break;
    case EventHints::ChangeColumn: handleChangeColumn(tname, cname);     break;
    case EventHints::ChangeRow:    handleChangeRow(tname, row);          break;
    case EventHints::ChangeData:   handleChangeData(tname, cname, row);  break;
    case EventHints::AddRow:       handleAddRow(tname, row);             break;
    case EventHints::InsertRow:    handleInsertRow(tname, row);          break;
    case EventHints::DeleteRow:    handleDeleteRow(tname, row);          break;
    default:
        spdlog::error("RTDM: unknown EventHint: {}", static_cast<int>(hint_data.hint));
        break;
    }
}

std::shared_ptr<QDataBlock>
RTablesDataManager::get(std::string tname, std::string Cols)
{
    /*
     * Перед получением таблицы из RTDM удалим таблицы на которые никто не ссылается,
     * например если таблицу открыли, а потом закрыли, тогда она остается в RTDM со счетчиком ссылок (use_count()) = 1
     * если не удалять, тогда при расчете УР придется обновлять данные, но необходимости в этом нет.
    */
    auto iter = mpTables.begin();
    auto endIter = mpTables.end();
    for(; iter != endIter; ) {
        if (iter->second.use_count() == 1) {
            qDebug()<<"RTDM: delete table with use_count() = 1 -> " <<iter->first.c_str();
            iter = mpTables.erase(iter);
        } else {
            qDebug()<<"RTDM: " << iter->first.c_str() <<" use_count() =  " <<iter->second.use_count();
            ++iter;
        }
    }

    auto it = mpTables.find(tname);
    if (it != mpTables.end() )
    {
        return it->second;
    }else{
        qDebug()<<"RTDM: add Table" << tname.c_str();
        mpTables.insert(std::make_pair(tname, std::make_shared<QDataBlock>()));
        getDataBlock(tname,Cols,*mpTables.find(tname)->second);
    }

    return mpTables.find(tname)->second;
}

long RTablesDataManager::column_index(std::string tname , std::string _col_name)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tindex{tablesx->FindIndex(tname)};
    if ( tindex.Value() < 0 )
        return -1;
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload res{columns->FindIndex(_col_name)};
    return res.Value();
}

void  RTablesDataManager::getDataBlock(std::string tname , std::string Cols , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);

    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };

    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}

void  RTablesDataManager::getDataBlock(std::string tname , std::string Cols , QDataBlock& QDB,FieldDataOptions Options )
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}

void  RTablesDataManager::getDataBlock(std::string tname , QDataBlock& QDB,FieldDataOptions Options )
{
    std::string Cols = getTCols(tname);
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}

void  RTablesDataManager::getDataBlock(std::string tname , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);

    getDataBlock(tname,QDB,Options);
}

std::string  RTablesDataManager::getTCols(std::string tname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload ColumnsCount{ columns->Count() };

    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Type = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Type))->String()).Value();
        std::string col_Name = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Name))->String()).Value();

        str_cols_.append(col_Name);
        str_cols_.append(",");
    }
    if(str_cols_.length()>0)
        str_cols_.pop_back();

    return str_cols_;
}
long  RTablesDataManager::getColIndex(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    long res = IRastrPayload(col->Index()).Value();

    return res;
}
ePropType  RTablesDataManager::getColType(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    ePropType res = IRastrPayload(col->Type()).Value();

    return res;
}

QDataBlock* RTablesDataManager::findCachedBlock(const std::string& tname)
{
    // Вспомогательный метод: все handle* вызывают его первым.
    // Если таблица не кешируется (не была открыта) — ничего обновлять не нужно.
    auto it = mpTables.find(tname);
    return (it != mpTables.end()) ? it->second.get() : nullptr;
}

void RTablesDataManager::handleChangeAll()
{
    // Все таблицы изменились — типично при загрузке нового файла расчёта.
    // Сбрасываем и перечитываем каждый кешированный блок.
    /// @note сигналы (sig_BeginResetModel/sig_EndResetModel) могут изменять mpTables,
    /// поэтому только копирование
    for (auto [tname, sp_QDB] : mpTables)
    {
        emit sig_BeginResetModel(tname);
        sp_QDB->Clear();
        getDataBlock(tname, *sp_QDB);
        emit sig_EndResetModel(tname);
    }

    for (auto& p : mpTables) {
        qInfo() << "  -" << p.first.c_str() << "use_count=" << p.second.use_count();
    }
}

void RTablesDataManager::handleChangeTable(const std::string& tname)
{
    // Структура таблицы изменилась (добавлена/удалена колонка).
    // Необходима полная перезагрузка: и данных, и метаданных колонок.
    qInfo() << "ENTER handleChangeTable for table:" << tname.c_str();
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginResetModel(tname);

    //it->second->Clear();
    //GetDataBlock(tname,(*it->second.get()));
    //emit RTDM_UpdateModel(tname);

    // TO DO:
    // Вызвать чтение свойств столбцов
    // emit RTDM_UpdateProperties

    emit sig_EndResetModel(tname);
}

void RTablesDataManager::handleChangeColumn(const std::string& tname,
                                            const std::string& cname)
{
    // Изменились все значения одной колонки (например, после групповой коррекции).
    // Перечитываем только эту колонку, не трогая остальные.
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginResetModel(tname);

    IRastrTablesPtr tables{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table { tables->Item(tname) };

    // Читаем одну колонку в промежуточный блок
    DataBlock<FieldVariantData> col_block;
    IRastrResultVerify(table->DataBlock(cname, col_block));

    const long     col_ind  = getColIndex(tname, cname);
    const ePropType col_type = getColType(tname, cname);
    const long rows = static_cast<long>(pqdb->RowsCount());

    for (long i = 0; i < rows; ++i)
    {
        // Преобразуем вариант к нужному типу и записываем в кеш.
        switch (col_type)
        {
        case ePropType::Double:
            pqdb->Set(i, col_ind, std::visit(ToDouble(), col_block.Get(i, 0)));
            break;
        case ePropType::Int:
        case ePropType::Enum:
        case ePropType::Superenum:
        case ePropType::Enpic:
        case ePropType::Color:
            pqdb->Set(i, col_ind, std::visit(ToLong(), col_block.Get(i, 0)));
            break;
        case ePropType::String:
            pqdb->Set(i, col_ind, std::visit(ToString(), col_block.Get(i, 0)));
            break;
        case ePropType::Bool:
            ///@note возможно, всё таки надо через visit, прописав ToBool
            /// Если в variant в данный момент лежит не bool, а, например,
            /// long (даже если это 0 или 1) или double, программа выбросит
            /// исключение std::bad_variant_access и, если его не поймать, аварийно завершится.
            try
            {
                pqdb->Set(i, col_ind, std::get<bool>(col_block.Get(i, 0)));
            }catch(...){
                spdlog::error("Ошибка преобразования колонки bool для {} {}",
                              tname.c_str(),
                              cname.c_str());
            }
            break;
        default:
            spdlog::error("RTDM::handleChangeColumn: unknown col type for {}", cname.c_str());
            break;
        }
    }

    emit sig_EndResetModel(tname);
}

void RTablesDataManager::handleChangeRow(const std::string& tname, long row)
{
    // Изменились все колонки одной строки.
    // Перечитываем строку целиком через Column::Value().
    qInfo() << "ENTER handleChangeRow for table:" << tname.c_str();
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    IRastrTablesPtr tables  { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tables->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload colsCount { columns->Count() };

    const long ncols = colsCount.Value();
    for (long i = 0; i < ncols; ++i)
    {
        IRastrColumnPtr  col      { columns->Item(i) };
        IRastrVariantPtr val_ptr  { col->Value(row) };
        pqdb->Set(row, i, val_ptr);
    }

    // Точечный сигнал: изменилась только строка row, все колонки
    emit sig_dataChanged(tname, row, 0, row, ncols);
}

void RTablesDataManager::handleChangeData(const std::string& tname,
                                          const std::string& cname,
                                          long row)
{
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    const long pnparr_nrows = static_cast<long>(pqdb->RowsCount());
    const long pnparr_ncols = static_cast<long>(pqdb->ColumnsCount());

    const long col_ind = getColIndex(tname, cname);
    if (col_ind < 0 || col_ind >= pnparr_ncols) return;
    if (row  < 0 || row  >= pnparr_nrows)       return;

    IRastrTablesPtr  tables { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(tname) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col     { columns->Item(cname) };

    ///@note Читаем одно значение через Value() — безопасно даже для новых строк.
    /// DataBlock("", VDB) с пустым списком колонок падает сразу после AddRow.
    /// вместо DataBlock("", VDB) — читаем только ту колонку, которая изменилась (col->Value(row)).
    /// Это то, что делает handleChangeColumn, но для одной строки.
    /// Плагин нормально отдаёт одно значение через Value() даже для новой строки —
    /// а вот DataBlock с пустым списком колонок пытается вычислить все поля сразу,
    /// включая формульные, и на неустановившейся строке падает.
    IRastrVariantPtr v_ptr{ col->Value(row) };

    // определяем тип колонки и вызываем соответствующий типизированный метод.
    const ePropType col_type = getColType(tname, cname);
    switch (col_type){
    case ePropType::Double:{
        IRastrPayload<double> val{ v_ptr->Double() };
        pqdb->Set(row, col_ind, static_cast<double>(val.Value()));
        break;
    }
    case ePropType::Int: //Все эти типы обрабатываются одним блоком ниже
    case ePropType::Enum:
    case ePropType::Superenum:
    case ePropType::Enpic:
    case ePropType::Color:{
        IRastrPayload<long> val{ v_ptr->Long() };
        pqdb->Set(row, col_ind, static_cast<long>(val.Value()));
        break;
    }
    case ePropType::String:{
        IRastrPayload<const std::string> val{ v_ptr->String() };
        pqdb->Set(row, col_ind, std::string(val.Value()));
        break;
    }
    case ePropType::Bool:{
        IRastrPayload<bool> val{ v_ptr->Bool() };
        pqdb->Set(row, col_ind, val.Value());
        break;
    }
    default:
        qWarning() << "handleChangeData: unhandled type for col"
                   << cname.c_str();
        break;
    }

    ///@note добавлен вызов сигнала, уведомляющий об изменении по аналогии
    emit sig_dataChanged(tname, row, col_ind, row, col_ind);
}

void RTablesDataManager::handleAddRow(const std::string& tname, long row)
{
    // Строка добавлена в конец таблицы.
    // QDataBlock::AddRow() выделяет место под новую строку (данные будут 0/пусто).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginInsertRow(tname, row, row);
    pqdb->AddRow();
    emit sig_EndInsertRow(tname);
}

void RTablesDataManager::handleInsertRow(const std::string& tname, long row)
{
    // Строка вставлена в позицию row (InsertRow сдвигает остальные вниз).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginInsertRow(tname, row, row);
    pqdb->InsertRow(row);
    emit sig_EndInsertRow(tname);
}

void RTablesDataManager::handleDeleteRow(const std::string& tname, long row)
{
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    /// @todo убедиться, что BeginResetModel/EndResetModel сброс необходим
    emit sig_BeginResetModel(tname);
    pqdb->DeleteRow(row);
    emit sig_EndResetModel(tname);
}
