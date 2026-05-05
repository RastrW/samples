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

    m_str_cols.clear();

    for (const auto& cs : schema.columns) {
        // initialize() принимает const& — объект ColumnSchema не копируется.
        vCols_.push_back(cs.name);
        m_str_cols += cs.name + ",";

        RCol rc;
        rc.initialize(cs);
        emplace_back(std::move(rc));

        mCols_.try_emplace(cs.name, static_cast<int>(cs.index));
    }

    // Скрываем колонки, не входящие в форму.
    // Скрытые колонки присутствуют в datablock — просто не показываются в UI.
    std::unordered_set<std::string> formCols;
    for (const auto& f : form.Fields())
        formCols.insert(f.Name());

    for (RCol& rc : *this)
        if (formCols.count(rc.getColName()))
            rc.setHidden(false);

    if (!m_str_cols.empty())
        m_str_cols.pop_back();

    spdlog::debug("RData: table={} columns={}", t_name_, schema.columns.size());
}

void RData::populateBlock(std::shared_ptr<ITableRepository> tables)
{
    const std::string visCols = get_cols(/*visible=*/true); // только не-hidden
    datablock = tables->getBlock(t_name_, visCols);
    rebuildBlockIndexMap();
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

FieldVariantData
RData::getCell(int rdataPos, int row) const
{
    const int blockCol = blockColIndex(rdataPos);
    if (blockCol < 0) return {};          // не загружена
    return datablock->Get(row, blockCol);
}

std::string RData::get_cols(bool visible) const
{
    std::string ret;
    for (const RCol& rc : *this) {
        if (!visible || !rc.isHidden())
            ret += rc.getColName() + ",";
    }
    if (!ret.empty())
        ret.pop_back();
    return ret;
}
