#include "rdata.h"

#include "UIForms.h"
#include "rdata.h"
#include <unordered_set>

RData::RData(const ITableRepository::TableSchema& schema,
             const CUIForm&                        form){

    // schema передан по const& — объект не копируется.
    // Строки (name, title) копируются по одному разу в члены RData/RCol

    t_name_  = schema.name;
    t_title_ = schema.title;

    const long n_reserve = static_cast<long>(schema.columns.size()) + 5;
    reserve(n_reserve);
    // reserve() вызван ДО push_back, иначе при reallocation
    // итераторы инвалидируются и RCol теряют данные.
    ModelColumn col{0};
    for (const auto& cs : schema.columns) {
        RCol rc;
        rc.initialize(cs);
        emplace_back(std::move(rc));
        mCols_.try_emplace(cs.name, col);
        ++col;
    }
    // Скрываем колонки, не входящие в форму.
    // Скрытые колонки присутствуют в datablock — просто не показываются в UI.
    std::unordered_set<std::string> formCols;

    for (const auto& f : form.Fields()) {
        //if ((t_name_ == "vetv" && f.Name() == "name"))
            //continue; // игнорируем только для vetv
        formCols.insert(f.Name());
    }

    for (RCol& rc : *this)
        if (formCols.count(rc.getColName()))
            rc.setHidden(false);
    spdlog::debug("RData: table={} columns={}", t_name_, schema.columns.size());
}

void RData::populateBlock(std::shared_ptr<ITableRepository> tables){

    const std::string visCols = get_cols(/*visible=*/true); // только не-hidden
    datablock = tables->getBlock(t_name_, visCols);
    rebuildBlockIndexMap();

    auto* db = datablock.get();

    spdlog::info("[PERF] rows = {}", db->RowsCount());
    spdlog::info("[PERF] cols = {}", db->ColumnsCount());
    spdlog::info("[PERF] datasize = {}", db->DataSize());
    spdlog::info("[PERF] bytes = {}", db->DataSize() * sizeof(FieldVariantData));
}

void RData::rebuildBlockIndexMap()
{
    m_blockColIdx.assign(size(), LocalIndex{});
    if (!datablock) return;
    for (size_t i = 0; i < size(); ++i)
        m_blockColIdx[i] =
            LocalIndex{datablock->localColumnIndex((*this)[i].getColName())};
}

LocalIndex
RData::ensureLoaded(ModelColumn pos,
                    std::shared_ptr<ITableRepository> tables) const
{
    if (!pos.valid_in(size())) return {};

    const std::string& name = (*this)[pos.to_size()].getColName();
    tables->ensureColumn(t_name_, name);
    updateBlockIndex(pos);
    return localIndexOf(pos);
}

FieldVariantData
RData::getCell(ModelColumn pos, int row) const
{
    const LocalIndex li = localIndexOf(pos);
    if (li.invalid()) return {};
    return datablock->Get(row, li.value);
}

void RData::updateBlockIndex(ModelColumn pos) const noexcept
{
    if (!datablock || !pos.valid_in(m_blockColIdx.size())) return;

    const std::string& name = (*this)[pos.to_size()].getColName();
    m_blockColIdx[pos.to_size()] =
        LocalIndex{datablock->localColumnIndex(name)};
}

std::string RData::get_cols(bool visible) const{
    // Резервируем примерный размер: N колонок × ~5 символов + запятые
    std::string ret;
    ret.reserve(size() * 6);

    for (const RCol& rc : *this) {
        if (!visible || !rc.isHidden()) {
            if (!ret.empty()) ret += ',';
            ret += rc.getColName();        // без временной строки
        }
    }
    return ret;
}

std::vector<std::string> RData::colNames() const {
    std::vector<std::string> names;
    names.reserve(size());
    for (const RCol& rc : *this)
        names.push_back(rc.getColName());
    return names;
}

ModelColumn
RData::modelColumnOf(const std::string& colName) const noexcept{
    auto it = mCols_.find(colName);
    return it != mCols_.end() ? ModelColumn{it->second} : ModelColumn{};
}

AstraIndex
RData::astraIndexOf(ModelColumn pos) const noexcept{
    if (!pos.valid_in(size())) return {};
    return (*this)[pos.to_size()].astraIndex();
}

LocalIndex
RData::localIndexOf(ModelColumn pos) const noexcept{
    if (!pos.valid_in(m_blockColIdx.size())) return {};
    return LocalIndex{m_blockColIdx[pos.to_size()]};
}

const RCol* RData::colAt(ModelColumn pos) const noexcept{
    if (!pos.valid_in(size())) return nullptr;
    return &(*this)[pos.to_size()];
}