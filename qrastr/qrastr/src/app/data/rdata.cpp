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

        AddCol(rc);

        if (mCols_.find(cs.name) == mCols_.end())
            mCols_.insert({cs.name, static_cast<int>(cs.index)});
    }

    // Скрываем колонки, не входящие в форму.
    // Скрытые колонки присутствуют в pnparray_ — просто не показываются в UI.
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

int RData::AddCol(const RCol& rcol)
{
    emplace_back(rcol);
    return static_cast<int>(size());
}

void RData::populateBlock(ITableRepository* repo)
{
    // repo — невладеющий указатель, время жизни гарантировано RtabController.
    // getBlock возвращает shared_ptr — счётчик ссылок увеличивается,
    // данные не копируются.
    pnparray_ = repo->getBlock(t_name_, m_str_cols);
}

std::string RData::getCommaSeparatedFieldNames() const
{
    std::string ret;
    for (const RCol& rc : *this)
        ret += rc.getColName() + ",";
    if (!ret.empty())
        ret.pop_back();
    return ret;
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
