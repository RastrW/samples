#include "rTablesDataAdapter.h"
#include "qastra.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "UIForms.h"
#include "QDataBlocks.h"
#include <memory>
#include <QFileInfo>
#include <QElapsedTimer>
#include "tableIndexTypes.h"

RTablesDataAdapter::RTablesDataAdapter(std::shared_ptr<QAstra> _pqastra) :
    m_pqastra(_pqastra)
{
    connect(m_pqastra.get(), &QAstra::onRastrHint,
            this, &RTablesDataAdapter::slot_rastrHint);
}

ITableRepository::TableSchema
RTablesDataAdapter::getSchema(const std::string& tname)
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

std::shared_ptr<QDataBlock>
RTablesDataAdapter::getBlock(const std::string& tname, const std::string& cols)
{
    QElapsedTimer t; t.start();
    /*
     * Перед получением таблицы из RTDA удалим таблицы на которые никто не ссылается,
     * например если таблицу открыли, а потом закрыли, тогда она остается в RTDA со счетчиком ссылок (use_count()) = 1
     * если не удалять, тогда при расчете УР придется обновлять данные, но необходимости в этом нет.
    */
    for (auto it = mpTables.begin(); it != mpTables.end(); ) {
        if (it->second.use_count() == 1) {
            spdlog::debug ("RTDA: delete table with use_count() = 1 -> {}", it->first);
            it = mpTables.erase(it);
        } else {
            spdlog::debug("RTDA: {} use_count() =  {}", it->first, it->second.use_count());
            ++it;
        }
    }

    auto [it, inserted] = mpTables.emplace(tname, nullptr);
    if (inserted) {
        it->second = std::make_shared<QDataBlock>();
        fillBlock(tname, *it->second, cols);
        spdlog::info("RTDA: add Table {}", tname.c_str());
    }
    return it->second;
}

void RTablesDataAdapter::ensureColumn(const std::string& tname,
                                      const std::string& colName)
{
    auto it = mpTables.find(tname);
    if (it == mpTables.end()) return;       // таблица не кеширована

    QDataBlock& block = *it->second;
    if (block.localColumnIndex(colName) >= 0) return; // уже есть

    spdlog::debug("RTDA: lazy-load column '{}' for table '{}'", colName, tname);
    QDataBlock colBlock;
    fillBlock(tname, colBlock, colName);    // 1 колонка × N строк
    block.mergeColumn(colName, colBlock);

    // Карта индексов в RData обновится
}

void RTablesDataAdapter::fillBlock(const std::string& tname,
                                   QDataBlock& qdb,
                                   const std::string& cols,
                                   const std::optional<FieldDataOptions>& opts)
{
    QElapsedTimer t; t.start();
    // Дефолтные опции
    FieldDataOptions options;
    if (opts) {
        options = *opts;
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
    spdlog::debug("[PERF] fill QDataBlock: {} ms", t.restart());
}

void RTablesDataAdapter::getDynamicForms(std::vector<CUIForm>& out)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload   cnt    { tablesx->Count() };

    for (int i = 0; i < cnt.Value(); ++i) {
        IRastrTablePtr  table     { tablesx->Item(i) };
        IRastrPayload   tab_name  { table->Name() };
        IRastrPayload   templ_name{ table->TemplateName() };
        IRastrPayload   tab_desc  { table->Description() };

        const std::string str_templ = templ_name.Value();

        // Проверяем, что шаблон таблицы - это .form файл
        if (QFileInfo(QString::fromStdString(str_templ)).suffix() != "form")
            continue;
        // Создаём динамическую форму
        CUIForm form;
        form.SetName(tab_desc.Value());
        form.SetTableName(tab_name.Value());
        // Определяем вертикальность (если 1 строка)
        IRastrColumnsPtr columns{ table->Columns() };
        const size_t nrows = IRastrPayload{ table->Size()    }.Value();
        const size_t ncols = IRastrPayload{ columns->Count() }.Value();

        if (nrows == 1)
            form.SetVertical(true);
        // Добавляем поля из колонок
        for (size_t j = 0; j < ncols; ++j) {
            IRastrColumnPtr col     { columns->Item(j) };
            IRastrPayload   col_name{ col->Name()      };
            form.Fields().emplace_back(col_name.Value());
        }

        out.push_back(std::move(form));
    }
}

std::vector<long>
RTablesDataAdapter::rowsBySelection(const std::string& tname,
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

void RTablesDataAdapter::setValue(const std::string&      tname,
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

long RTablesDataAdapter::columnIndex(const std::string& tname,
                                     const std::string& colName)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    const long tindex = IRastrPayload{ tablesx->FindIndex(tname) }.Value();
    if (tindex < 0) return -1;

    IRastrTablePtr   table  { tablesx->Item(tindex) };
    IRastrColumnsPtr columns{ table->Columns() };
    return IRastrPayload{ columns->FindIndex(colName) }.Value();
}

std::string RTablesDataAdapter::getTCols(const std::string& tname)
{
    IRastrTablesPtr  tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr   table   { tablesx->Item(tname) };
    IRastrColumnsPtr columns { table->Columns() };
    const long       count   = IRastrPayload{ columns->Count() }.Value();

    std::string result;
    result.reserve(static_cast<size_t>(count) * 5); // примерный размер
    // Берем все колонки таблицы
    for (long i = 0; i < count; ++i) {
        if (i > 0) result += ',';
        IRastrColumnPtr col     { columns->Item(i) };
        result += IRastrPayload{ col->Name() }.Value();
    }
    return result;
}

long
RTablesDataAdapter::getColIndex(const std::string& tname,
                                      const std::string& cname)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };

    return IRastrPayload(col->Index()).Value();
}

QDataBlock*
RTablesDataAdapter::findCachedBlock(const std::string& tname){
    // Вспомогательный метод: все handle* вызывают его первым.
    // Если таблица не кешируется (не была открыта) — ничего обновлять не нужно.
    auto it = mpTables.find(tname);
    return (it != mpTables.end()) ? it->second.get() : nullptr;
}

void
RTablesDataAdapter::slot_rastrHint(const _hint_data& hint_data)
{
    long row = hint_data.n_indx;
    const auto&  cname = hint_data.str_column;
    const auto&  tname = hint_data.str_table;

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
        spdlog::error("RTDA: unknown EventHint: {}", static_cast<int>(hint_data.hint));
        break;
    }
}

void RTablesDataAdapter::handleChangeAll()
{
    // Все таблицы изменились — типично при загрузке нового файла расчёта.
    spdlog::debug("handleChangeAll");
    std::vector<std::string> keys;
    for (auto& [k, v] : mpTables)
        //только живые таблицы
        if (v.use_count() > 1) {keys.push_back(k);}
    // Сбрасываем и перечитываем каждый кешированный блок.
    for (const auto& tname : keys) {
        auto it = mpTables.find(tname);
        if (it == mpTables.end()) continue;

        reloadBlock(tname, it->second);
    }
}

void RTablesDataAdapter::handleChangeTable(const std::string& tname){
    spdlog::debug("handleChangeTable tname={}", tname);
    auto it = mpTables.find(tname);
    if (it == mpTables.end()) return;
    reloadBlock(tname, it->second);
}

void RTablesDataAdapter::handleChangeColumn(const std::string& tname,
                                            const std::string& cname)
{
    spdlog::debug("handleChangeColumn tname={} cname={}", tname, cname);
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    // LocalIndex — только чтобы проверить, загружена ли колонка
    const LocalIndex li{ pqdb->localColumnIndex(cname) };
    if (li.invalid()) return;  // не загружена — обновление не нужно

    QDataBlock colBlock;
    // все строки одной колонки
    fillBlock(tname, colBlock, cname);

    const long nRows = static_cast<long>(pqdb->RowsCount());
    for (long row = 0; row < nRows; ++row)
        pqdb->Set(row, li.value, colBlock.Get(row, 0));

    if (nRows > 0)
        emit sig_dataChanged(tname, 0, cname, static_cast<int>(nRows - 1), cname);
}

void RTablesDataAdapter::handleChangeRow(const std::string& tname, long row)
{
    spdlog::debug("handleChangeRow tname={} row={}", tname, row);
    // Изменились все колонки одной строки.
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;
    // Читаем только то, что загружено
    const std::string loadedCols = pqdb->Columns();
    QDataBlock rowBlock;
    fillBlock(tname, rowBlock, loadedCols);
    // Копируем только нужную строку
    const int ncols = static_cast<int>(pqdb->ColumnsCount());
    for (int li = 0; li < ncols; ++li) {
        // имя по локальному индексу в кеше
        const std::string name = pqdb->columnName(li);
         // ← индекс в rowBlock
        const long srcLi = rowBlock.localColumnIndex(name);
        if (srcLi < 0) continue;
        pqdb->Set(row, li, rowBlock.Get(row, srcLi));
    }

    // Пустые строки = "все колонки" — RModel интерпретирует сам
    emit sig_dataChanged(tname,
                         static_cast<int>(row), /*colFrom=*/"",
                         static_cast<int>(row), /*colTo=*/"");
}

void RTablesDataAdapter::handleChangeData(const std::string& tname,
                                          const std::string& cname,
                                          long row)
{
    spdlog::debug("handleChangeData tname={} col={} row={}", tname, cname, row);
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    // LocalIndex нужен только чтобы записать в блок
    const LocalIndex li{ pqdb->localColumnIndex(cname) };
    // если загружена — обновляем
    if (li.valid() && row >= 0 && row < static_cast<long>(pqdb->RowsCount()))
        pqdb->Set(row, li.value, m_pqastra->GetVal(tname, cname, row));

    // Если не загружена — пропускаем запись в блок,
    // но сигнал всё равно шлём: View перечитает через lazy load
    emit sig_dataChanged(tname,
                         static_cast<int>(row), cname,
                         static_cast<int>(row), cname);
    emit sig_referenceChanged(tname);
}

void RTablesDataAdapter::handleAddRow(const std::string& tname, long row)
{
    spdlog::debug("handleAddRow tname={} row={}", tname, row);
    // Строка добавлена в конец таблицы.
    // QDataBlock::AddRow() выделяет место под новую строку (данные будут 0/пусто).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_beginInsertRow(tname, row, row);
    pqdb->AddRow();
    emit sig_endInsertRow(tname);
    // новая строка изменила справочник
    emit sig_referenceChanged(tname);
}

void RTablesDataAdapter::handleInsertRow(const std::string& tname, long row)
{
    spdlog::debug("handleInsertRow tname={} row={}", tname, row);

    // Строка вставлена в позицию row (InsertRow сдвигает остальные вниз).
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_beginInsertRow(tname, row, row);
    pqdb->InsertRow(row);
    emit sig_endInsertRow(tname);
    // новая строка изменила справочник
    emit sig_referenceChanged(tname);
}

void RTablesDataAdapter::handleDeleteRow(const std::string& tname, long row)
{
    spdlog::debug("handleDeleteRow tname={} row={}", tname, row);
    QDataBlock* pqdb = findCachedBlock(tname);
    if (!pqdb) return;

    emit sig_beginRemoveRows(tname, row, row);
    pqdb->DeleteRow(row);
    emit sig_endRemoveRows(tname);
    emit sig_referenceChanged(tname);
}

void RTablesDataAdapter::setColumnProperty(const std::string& tname,
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

void RTablesDataAdapter::calcColumn(const std::string& tname,
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

void RTablesDataAdapter::addRows(const std::string& tname, size_t count)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (size_t i = 0; i < count; ++i)
        IRastrPayload { table->AddRow() };
}

void RTablesDataAdapter::insertRows(const std::string& tname,
                                    long               startRow,
                                    int                count){
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify { table->InsertRow(startRow) };
}

void RTablesDataAdapter::deleteRows(const std::string& tname,
                                    long               startRow,
                                    int                count)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify { table->DeleteRow(startRow) };
}

void RTablesDataAdapter::duplicateRow(const std::string& tname, long row)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    IRastrResultVerify { table->DuplicateRow(row) };
}

long RTablesDataAdapter::tableSize(const std::string& tname)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    return static_cast<long>(IRastrPayload(table->Size()).Value());
}

void RTablesDataAdapter::setTableSize(const std::string& tname, long size)
{
    IRastrTablesPtr tablesx { m_pqastra->getRastr()->Tables() };
    IRastrTablePtr  table   { tablesx->Item(tname) };
    IRastrResultVerify { table->SetSize(static_cast<IndexT>(size)) };
}

void RTablesDataAdapter::exportToCsv(const std::string& tname,
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

void RTablesDataAdapter::importToCsv(const std::string& tname,
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

void RTablesDataAdapter::setLockEvent(bool lock) {
    IRastrResultVerify{m_pqastra->getRastr()->SetLockEvent(lock)};
}

bool RTablesDataAdapter::tableExists(const std::string& tname) {
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload   res    { tablesx->FindIndex(tname) };
    return res.Value() >= 0;
}

void RTablesDataAdapter::reloadBlock(const std::string& tname,
                                     std::shared_ptr<QDataBlock>& block)
{
    // что было загружено
    const std::string loadedCols = block->Columns();
    emit sig_beginResetModel(tname);
    block->Clear();
    // Перезагружаем данные И структуру
    if (!loadedCols.empty()){
         // только то же самое
        fillBlock(tname, *block, loadedCols);
    }
    emit sig_endResetModel(tname);
}