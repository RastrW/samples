#include "rtablesdatamanager.h"
#include "qastra.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "astra_headers/UIForms.h"
#include "QDataBlocks.h"

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
    for (CUIForm &form : *m_plstUIForms){
        if (stringutils::MkToUtf8(form.Name()) == _name){
            return &form;
        }
    }
    return nullptr;
}

void  RTablesDataManager::onRastrHint(const _hint_data& hint_data)
{
    long row = hint_data.n_indx;
    std::string cname = hint_data.str_column;
    std::string tname = hint_data.str_table;

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
            spdlog::info ("RTDM: delete table with use_count() = 1 -> {}", iter->first.c_str());
            iter = mpTables.erase(iter);
        } else {
           spdlog::info("RTDM: {} use_count() =  {}", iter->first.c_str(), iter->second.use_count());
            ++iter;
        }
    }

    auto it = mpTables.find(tname);
    if (it != mpTables.end() ){
        return it->second;
    }

    spdlog::info("RTDM: add Table {}", tname.c_str());
    mpTables.insert(std::make_pair(tname, std::make_shared<QDataBlock>()));
    getDataBlock(tname,Cols,*mpTables.find(tname)->second);

    return mpTables.find(tname)->second;
}

void RTablesDataManager::setValue(const std::string&      tname,
                                  const std::string&      cname,
                                  long                    row,
                                  const FieldVariantData& value)
{
    IRastrTablesPtr  tables  { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(tname) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(cname) };

    std::visit([&](const auto& v)
               {
                   using VT = std::decay_t<decltype(v)>;

                   if constexpr (std::is_same_v<VT, bool>)
                       IRastrResultVerify(col_ptr->SetValue(row, v));

                   else if constexpr (std::is_same_v<VT, long>)
                       IRastrResultVerify(col_ptr->SetValue(row, v));

                   else if constexpr (std::is_same_v<VT, double>)
                       IRastrResultVerify(col_ptr->SetValue(row, v));

                   else if constexpr (std::is_same_v<VT, uint64_t>)
                       // uint64_t → приводим к long явно, чтобы устранить неоднозначность
                       IRastrResultVerify(col_ptr->SetValue(row, static_cast<long>(v)));

                   else if constexpr (std::is_same_v<VT, std::string>)
                       IRastrResultVerify(col_ptr->SetValue(row, v));

                   // std::monostate — значение не задано, ничего не делаем

               }, value);
}

long RTablesDataManager::column_index(std::string tname , std::string _col_name)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tindex{tablesx->FindIndex(tname)};
    if ( tindex.Value() < 0 ){
        return -1;
    }
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
    for (long index{ 0 }; index < ColumnsCount.Value(); index++){
        IRastrColumnPtr col     { columns->Item(index) };
        std::string     col_Name = IRastrPayload(col->Name()).Value();
        str_cols_.append(col_Name);
        str_cols_.append(",");
    }

    if (!str_cols_.empty()){
        str_cols_.pop_back();
    }
    return str_cols_;
}

long  RTablesDataManager::getColIndex(std::string tname,std::string cname)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };

    return IRastrPayload(col->Index()).Value();
}

ePropType  RTablesDataManager::getColType(std::string tname,std::string cname)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };

    return IRastrPayload(col->Type()).Value();
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
    for (auto& [k, v] : mpTables){
        //только живые таблицы
        if (v.use_count() > 1){
            keys.push_back(k);
        }
    }
    // Сбрасываем и перечитываем каждый кешированный блок.
    for (const auto& tname : keys) {
        auto it = mpTables.find(tname);
        if (it == mpTables.end()){continue;}
        emit sig_BeginResetModel(tname);

        it->second->Clear();
        getDataBlock(tname, *it->second);

        emit sig_EndResetModel(tname);
    }
}

void RTablesDataManager::handleChangeTable(const std::string& tname)
{

    spdlog::info("ENTER handleChangeTable for table: {}", tname.c_str());
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginResetModel(tname);

    // Перезагружаем данные И структуру
    pqdb->Clear();
    getDataBlock(tname, *pqdb); // читает всё заново из плагина

    emit sig_EndResetModel(tname);
}

void RTablesDataManager::handleChangeColumn(const std::string& tname,
                                            const std::string& cname)
{
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    const long colIdx = getColIndex(tname, cname);
    if (colIdx < 0) return;

    // Один вызов вместо N — плагин сам копирует колонку блоком
    QDataBlock colBlock;
    getDataBlock(tname, cname, colBlock);

    const long nRows = static_cast<long>(pqdb->RowsCount());
    for (long row = 0; row < nRows; ++row)
        pqdb->Set(row, colIdx, colBlock.Get(row, 0));   // копируем из временного блока
    //Обновляем все строки только одного столбца
    if (nRows > 0)
        emit sig_dataChanged(tname, 0, colIdx, nRows - 1, colIdx);
}

void RTablesDataManager::handleChangeRow(const std::string& tname, long row)
{
    // Изменились все колонки одной строки.

    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    QDataBlock rowBlock;
    getDataBlock(tname, rowBlock);                     // все колонки, все строки
    // Копируем только нужную строку
    const long ncols = static_cast<long>(pqdb->ColumnsCount());
    for (long i = 0; i < ncols; ++i)
        pqdb->Set(row, i, rowBlock.Get(row, i));
    // изменились все колонки только одной строка row
    emit sig_dataChanged(tname, row, 0, row, ncols - 1);
}

void RTablesDataManager::handleChangeData(const std::string& tname,
                                          const std::string& cname,
                                          long row)
{
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb){ return;}

    const long colIdx = getColIndex(tname, cname);
    if (colIdx < 0 || colIdx >= static_cast<long>(pqdb->ColumnsCount())){return;}
    if (row  < 0 || row  >= static_cast<long>(pqdb->RowsCount())){return;}

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
