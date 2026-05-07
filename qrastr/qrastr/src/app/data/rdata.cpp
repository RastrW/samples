#include "rdata.h"

#include "UIForms.h"
#include "rdata.h"
#include <unordered_set>

RData::RData(const ITableRepository::TableSchema& schema,
             const CUIForm&                        form)
{
    // schema передан по const& — объект не копируется.
    // Строки (name, title) копируются по одному разу в члены RData/RCol

    t_name_  = schema.name;
    t_title_ = schema.title;

    const long n_reserve = static_cast<long>(schema.columns.size()) + 5;
    reserve(n_reserve);
    // reserve() вызван ДО push_back, иначе при reallocation
    // итераторы инвалидируются и RCol теряют данные.
    int rdataPos = 0;
    for (const auto& cs : schema.columns) {
        RCol rc;
        rc.initialize(cs);
        emplace_back(std::move(rc));
        mCols_.try_emplace(cs.name, rdataPos);
        ++rdataPos;
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

void RData::populateBlock(std::shared_ptr<ITableRepository> tables)
{
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
    m_blockColIdx.assign(size(), -1);
    if (!datablock) return;
    for (int i = 0; i < static_cast<int>(size()); ++i)
        m_blockColIdx[i] =
            static_cast<int>(datablock->localColumnIndex((*this)[i].getColName()));
}

int RData::blockColIndex(int rdataPos) const noexcept
{
    if (rdataPos < 0 || rdataPos >= static_cast<int>(m_blockColIdx.size()))
        return -1;
    return m_blockColIdx[rdataPos];
}

int RData::ensureBlockCol(int rdataPos,
                          std::shared_ptr<ITableRepository> tables) const
{
    if (rdataPos < 0 || rdataPos >= static_cast<int>(size())) return -1;
    const std::string& colName = (*this)[rdataPos].getColName();
    tables->ensureColumn(t_name_, colName);
    updateBlockIndex(rdataPos);
    return m_blockColIdx[rdataPos];
}

void RData::updateBlockIndex(int rdataPos) const noexcept
{
    if (!datablock) return;
    if (rdataPos < 0 || rdataPos >= static_cast<int>(m_blockColIdx.size()))
        return;
    m_blockColIdx[rdataPos] =
        static_cast<int>(datablock->localColumnIndex((*this)[rdataPos].getColName()));
}

FieldVariantData
RData::getCell(int rdataPos, int row) const
{
    const int blockCol = blockColIndex(rdataPos);
    if (blockCol < 0) return {};          // не загружена
    return datablock->Get(row, blockCol);
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

const std::vector<std::string>& RData::colNames() const {
    static std::vector<std::string> names;
    names.reserve(size());
    for (const RCol& rc : *this)
        names.push_back(rc.getColName());
    return names;
}

