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
    /// @note сигналы (sig_BeginResetModel/sig_EndResetModel) могут изменять mpTables,
    /// поэтому только копирование сначала собираем список ключей:
    std::vector<std::string> keys;
    for (auto& [k, v] : mpTables) keys.push_back(k);
    // Сбрасываем и перечитываем каждый кешированный блок.
    for (const auto& tname : keys) {
        auto it = mpTables.find(tname);
        if (it == mpTables.end()) continue;
        emit sig_BeginResetModel(tname);
        it->second->Clear();
        getDataBlock(tname, *it->second);
        emit sig_EndResetModel(tname);
    }
}

void RTablesDataManager::handleChangeTable(const std::string& tname)
{
    ///@todo не понятно, как добиться вызова этого события
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

    const long colIdx = getColIndex(tname, cname);
    if (colIdx < 0) return;

    emit sig_BeginResetModel(tname);

    const long nRows = static_cast<long>(pqdb->RowsCount());
    for (long row = 0; row < nRows; ++row)
        pqdb->Set(row, colIdx, m_pqastra->GetVal(tname, cname, row));

    emit sig_EndResetModel(tname);
}

void RTablesDataManager::handleChangeRow(const std::string& tname, long row)
{
    // Изменились все колонки одной строки.
    // Перечитываем строку целиком через Column::Value().
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    IRastrTablesPtr tables  { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tables->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    const long ncols = IRastrPayload{ columns->Count() }.Value();

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

    const long colIdx = getColIndex(tname, cname);
    if (colIdx < 0 || colIdx >= static_cast<long>(pqdb->ColumnsCount())) return;
    if (row  < 0 || row  >= static_cast<long>(pqdb->RowsCount()))        return;

    ///@note устанавливаем значение только для конкретной ячейки, т.к.
    /// Ранее была установка DataBlock("", VDB) с пустым списком колонок, он падает сразу после AddRow.
    pqdb->Set(row, colIdx, m_pqastra->GetVal(tname, cname, row));

    ///@note добавлен вызов сигнала, уведомляющий об изменении по аналогии
    emit sig_dataChanged(tname, row, colIdx, row, colIdx);
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

    emit sig_BeginRemoveRows(tname, row, row);
    pqdb->DeleteRow(row);
    emit sig_EndRemoveRows(tname);
}
