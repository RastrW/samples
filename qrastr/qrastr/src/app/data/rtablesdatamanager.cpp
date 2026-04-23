#include "rtablesdatamanager.h"
#include "qastra.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "UIForms.h"
#include "QDataBlocks.h"
#include <memory>

void RTablesDataManager::setQAstra( QAstra* _pqastra)
{
    m_pqastra = _pqastra;
    connect(m_pqastra, &QAstra::onRastrHint, this, &RTablesDataManager::onRastrHint);
}

void  RTablesDataManager::setForms ( std::list<CUIForm>* _lstUIForms)
{
    m_plstUIForms = _lstUIForms;
}

CUIForm*  RTablesDataManager::getForm ( std::string name)
{
    for (CUIForm &form : *m_plstUIForms){
        if (stringutils::MkToUtf8(form.Name()) == name){
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
RTablesDataManager::getBlock(const std::string& tname, const std::string& cols)
{
    /*
     * Перед получением таблицы из RTDM удалим таблицы на которые никто не ссылается,
     * например если таблицу открыли, а потом закрыли, тогда она остается в RTDM со счетчиком ссылок (use_count()) = 1
     * если не удалять, тогда при расчете УР придется обновлять данные, но необходимости в этом нет.
    */
    for (auto it = mpTables.begin(); it != mpTables.end(); ) {
        if (it->second.use_count() == 1) {
            spdlog::info ("RTDM: delete table with use_count() = 1 -> {}", it->first.c_str());
            it = mpTables.erase(it);
        } else {
            spdlog::info("RTDM: {} use_count() =  {}", it->first.c_str(), it->second.use_count());
            ++it;
        }
    }

    auto [it, inserted] = mpTables.emplace(tname, nullptr);
    if (inserted) {
        it->second = std::make_shared<QDataBlock>();
        fillBlock(tname, *it->second, cols);  // одна функция
        spdlog::info("RTDM: add Table {}", tname.c_str());
    }
    return it->second;
}

std::vector<long> RTablesDataManager::rowsBySelection(const std::string& tname,
                                                         const std::string& selection)
{
    std::vector<long> indices;
    if (selection.empty()) return indices;
    // Передаём строку выборки в плагин
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(tname) };
    IRastrResultVerify(table->SetSelection(selection));
    // Получаем индексы строк, прошедших выборку
    DataBlock<FieldVariantData> variantBlock;
    const IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variantBlock));

    indices = variantBlock.IndexesVector();
    return indices;
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

    spdlog::debug("setValue tname={} col={} row={}", tname, cname, row);

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

long RTablesDataManager::columnIndex(const std::string& tname,
                                     const std::string& colName)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tindex{tablesx->FindIndex(tname)};
    if ( tindex.Value() < 0 ){
        return -1;
    }
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload res{columns->FindIndex(colName)};
    return res.Value();
}

void RTablesDataManager::fillBlock(const std::string& tname,
                                      QDataBlock& qdb,
                                      const std::string& cols,
                                      std::optional<FieldDataOptions> opts)
{
    // Дефолтные опции — те же, что раньше были во всех перегрузках
    FieldDataOptions options;
    if (opts.has_value()) {
        options = opts.value();
    } else {
        options.SetEnumAsInt(TriBool::True);
        options.SetSuperEnumAsInt(TriBool::True);
        options.SetUseChangedIndices(true);
    }

    // Если список колонок не задан — берём все колонки таблицы
    const std::string& actualCols = cols.empty() ? getTCols(tname) : cols;

    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(tname) };
    IRastrResultVerify(table->DataBlock(actualCols, qdb, options));
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

QDataBlock* RTablesDataManager::findCachedBlock(const std::string& tname)
{
    // Вспомогательный метод: все handle* вызывают его первым.
    // Если таблица не кешируется (не была открыта) — ничего обновлять не нужно.
    auto it = mpTables.find(tname);
    return (it != mpTables.end()) ? it->second.get() : nullptr;
}

void RTablesDataManager::handleChangeAll()
{
    spdlog::debug("handleChangeAll");
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
        fillBlock(tname, *it->second);

        emit sig_EndResetModel(tname);
    }
}

void RTablesDataManager::handleChangeTable(const std::string& tname)
{
    spdlog::debug("handleChangeTable tname={}", tname);

    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginResetModel(tname);

    // Перезагружаем данные И структуру
    pqdb->Clear();
    fillBlock(tname, *pqdb); // читает всё заново из плагина

    emit sig_EndResetModel(tname);
}

void RTablesDataManager::handleChangeColumn(const std::string& tname,
                                            const std::string& cname)
{
    spdlog::debug("handleChangeColumn tname={} cname={}", tname, cname);
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    const long colIdx = getColIndex(tname, cname);
    if (colIdx < 0) return;

    // Один вызов вместо N — плагин сам копирует колонку блоком
    QDataBlock colBlock;
    // все колонки, но только одна строка
    fillBlock(tname, colBlock, cname);

    const long nRows = static_cast<long>(pqdb->RowsCount());
    for (long row = 0; row < nRows; ++row)
        pqdb->Set(row, colIdx, colBlock.Get(row, 0));   // копируем из временного блока
    //Обновляем все строки только одного столбца
    if (nRows > 0)
        emit sig_dataChanged(tname, 0, colIdx, nRows - 1, colIdx);
}

void RTablesDataManager::handleChangeRow(const std::string& tname, long row)
{
    spdlog::debug("handleChangeRow tname={} row={}", tname, row);
    // Изменились все колонки одной строки.
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    // Формируем строку из всех колонок таблицы
    const std::string cols = getTCols(tname);

    QDataBlock rowBlock;
    // все колонки, но только одна строка
    fillBlock(tname, rowBlock, cols);
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
    spdlog::debug("handleChangeData tname={} col={} row={}", tname, cname, row);
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
    emit sig_ReferenceChanged(tname);
}

void RTablesDataManager::handleAddRow(const std::string& tname, long row)
{
    spdlog::debug("handleAddRow tname={} row={}", tname, row);
    // Строка добавлена в конец таблицы.
    // QDataBlock::AddRow() выделяет место под новую строку (данные будут 0/пусто).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginInsertRow(tname, row, row);
    pqdb->AddRow();
    emit sig_EndInsertRow(tname);
    // новая строка изменила справочник
    emit sig_ReferenceChanged(tname);
}

void RTablesDataManager::handleInsertRow(const std::string& tname, long row)
{
    spdlog::debug("handleInsertRow tname={} row={}", tname, row);

    // Строка вставлена в позицию row (InsertRow сдвигает остальные вниз).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginInsertRow(tname, row, row);
    pqdb->InsertRow(row);
    emit sig_EndInsertRow(tname);
    // новая строка изменила справочник
    emit sig_ReferenceChanged(tname);
}

void RTablesDataManager::handleDeleteRow(const std::string& tname, long row)
{
    spdlog::debug("handleDeleteRow tname={} row={}", tname, row);
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_BeginRemoveRows(tname, row, row);
    pqdb->DeleteRow(row);
    emit sig_EndRemoveRows(tname);
    emit sig_ReferenceChanged(tname);
}

ITableRepository::TableSchema
RTablesDataManager::getSchema(const std::string& tname)
{
    TableSchema schema;
    schema.name = tname;

    IRastrTablesPtr  tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tablesx->Item(tname) };
    IRastrColumnsPtr columns { table->Columns() };

    schema.title = IRastrPayload(table->Description()).Value();

    const long count = IRastrPayload(columns->Count()).Value();
    schema.columns.reserve(count);

    auto prop = [&](IRastrColumnPtr& col, FieldProperties p) -> std::string {
        return IRastrPayload(
                   IRastrVariantPtr(col->Property(p))->String()
                   ).Value();
    };

    for (long i = 0; i < count; ++i) {
        IRastrColumnPtr col { columns->Item(i) };

        ColumnSchema cs;
        cs.tableName   = tname;
        cs.index       = i;
        cs.name        = IRastrPayload(col->Name()).Value();
        cs.title       = prop(col, FieldProperties::Title);
        cs.description = prop(col, FieldProperties::Description);
        cs.unit        = prop(col, FieldProperties::Unit);
        cs.width       = prop(col, FieldProperties::Width);
        cs.prec        = prop(col, FieldProperties::Precision);
        cs.expression  = prop(col, FieldProperties::Expression);
        cs.nameRef     = prop(col, FieldProperties::NameRef);
        cs.afor        = prop(col, FieldProperties::AFOR);
        cs.ff          = prop(col, FieldProperties::IsActiveFormula);
        cs.min         = prop(col, FieldProperties::Min);
        cs.max         = prop(col, FieldProperties::Max);
        cs.scale       = prop(col, FieldProperties::Scale);
        cs.cache       = prop(col, FieldProperties::Cache);

        try {
            cs.comPropTT = static_cast<enComPropTT>(
                std::stoi(prop(col, FieldProperties::Type)));
        } catch (...) {
            cs.comPropTT = enComPropTT::COM_PR_INT;
            spdlog::warn("getSchema: не удалось разобрать Type для колонки {}", cs.name);
        }

        schema.columns.push_back(std::move(cs));
    }

    return schema;
}

void RTablesDataManager::setColumnProperty(const std::string& tname,
                                           const std::string& colName,
                                           FieldProperties    prop,
                                           const std::string& value)
{
    IRastrTablesPtr  tables  { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(tname) };
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(colName) };
    IRastrResultVerify { col_ptr->SetProperty(prop, value) };
}

void RTablesDataManager::calcColumn(const std::string& tname,
                const std::string& colName,
                const std::string& expression,
                const std::string& selection) {

    IRastrTablesPtr  tables  { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tables->Item(tname) };
    IRastrResultVerify(table->SetSelection(selection));
    IRastrColumnsPtr columns { table->Columns() };
    IRastrColumnPtr  col_ptr { columns->Item(colName) };
    IRastrResultVerify(col_ptr->Calculate(expression));
}

void RTablesDataManager::addRows(const std::string& tname, size_t count)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (size_t i = 0; i < count; ++i)
        IRastrPayload { table->AddRow() };
}

void RTablesDataManager::insertRows(const std::string& tname,
                                    long               startRow,
                                    int                count){
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify { table->InsertRow(startRow) };
}

void RTablesDataManager::deleteRows(const std::string& tname,
                                    long               startRow,
                                    int                count)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify { table->DeleteRow(startRow) };
}

void RTablesDataManager::duplicateRow(const std::string& tname, long row)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    IRastrResultVerify { table->DuplicateRow(row) };
}

long RTablesDataManager::tableSize(const std::string& tname)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    return static_cast<long>(IRastrPayload(table->Size()).Value());
}

void RTablesDataManager::setTableSize(const std::string& tname, long size)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    IRastrResultVerify { table->SetSize(static_cast<IndexT>(size)) };
}

void RTablesDataManager::exportToCsv(const std::string& tname,
                                   const std::string& cols,
                                   const std::string& selection,
                                   const std::string& path,
                                   const std::string& divider,
                                   eCSVCode           mode)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    if (!selection.empty())
        IRastrResultVerify { table->SetSelection(selection) };
    IRastrResultVerify { table->ToCsv(mode, path, cols, divider) };
}

void RTablesDataManager::importToCsv(const std::string& tname,
                                     const std::string& cols,
                                     const std::string& selection,
                                     const std::string& file,
                                     const std::string& divider,
                                     const std::string& byDefault,
                                     eCSVCode           mode)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    IRastrResultVerify { table->ReadCsv(mode, file, cols, divider, byDefault) };
}

void RTablesDataManager::setLockEvent(bool lock) {
    IRastrResultVerify{m_pqastra->getRastr()->SetLockEvent(lock)};
}