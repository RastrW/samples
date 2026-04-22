#include "backInfoCache.h"
#include "rtablesdatamanager.h"
#include "rdata.h"
#include "QDataBlocks.h"
#include "utils.h"
#include <spdlog/spdlog.h>

void BackInfoCache::clear()
{
    m_enum.clear();
    m_nameref.clear();
    m_superenum.clear();
    m_pictureEnums.clear();
    m_namerefSources.clear();
    m_superenumSources.clear();
}

void BackInfoCache::rebuild(const RData& rdata, RTablesDataManager* pRTDM)
{
    clear();

    for (const RCol& rcol : rdata)
    {
        switch (rcol.getComPropTT())
        {
        case enComPropTT::COM_PR_ENUM:
            loadEnum(rcol);
            break;

        case enComPropTT::COM_PR_SUPERENUM:
            loadSuperenum(rcol, pRTDM);
            break;

        case enComPropTT::COM_PR_INT:
            loadNameref(rcol, pRTDM);
            break;

        case enComPropTT::COM_PR_ENPIC:
            loadEnpic(rcol);
            break;

        default:
            break;
        }
    }
}

void BackInfoCache::loadEnum(const RCol& rcol)
{
    const size_t idx = static_cast<size_t>(rcol.getIndex());
    const auto& nameRef = rcol.getNameRef();

    QStringList list;
    for (const auto& val : split(nameRef, '|'))
        list.append(QString::fromStdString(val));

    m_enum.emplace(idx, std::move(list));
}

void BackInfoCache::loadSuperenum(const RCol& rcol, RTablesDataManager* pRTDM)
{
    if (rcol.getNameRef().empty() || !pRTDM) return;

    const auto parts = parseSuperenumStr(rcol.getNameRef());
    if (!parts) return;

    // Проверяем наличие колонок в таблице-источнике прежде чем строить карту.
    // column_index возвращает < 0, если колонка не найдена.
    if (pRTDM->column_index(parts->srcTable, parts->keyCol)   < 0 ||
        pRTDM->column_index(parts->srcTable, parts->valueCol) < 0)
        return;

    const size_t idx = static_cast<size_t>(rcol.getIndex());
    m_superenum.emplace(idx, buildIdNameMap(pRTDM, parts->srcTable,
                                            parts->keyCol, parts->valueCol));
    m_superenumSources[parts->srcTable].push_back(idx);
}

void BackInfoCache::loadNameref(const RCol& rcol, RTablesDataManager* pRTDM)
{
    if (rcol.getNameRef().empty() || !pRTDM) return;

    const auto parts = parseNamerefStr(rcol.getNameRef());
    if (!parts) return;

    const size_t idx = static_cast<size_t>(rcol.getIndex());
    const long nameIdx = pRTDM->column_index(parts->srcTable, "name");
    const std::string valueCol = (nameIdx > -1) ? "name" : parts->keyCol;

    m_nameref.emplace(idx, buildIdNameMap(pRTDM, parts->srcTable,
                                          parts->keyCol, valueCol));
    m_namerefSources[parts->srcTable].push_back(idx);
}

void BackInfoCache::loadEnpic(const RCol& rcol)
{
    const size_t idx = static_cast<size_t>(rcol.getIndex());
    const auto& nameRef = rcol.getNameRef();

    const std::map<int, int> iconMap = parseEnpicNameref(nameRef);
    PictureList items;

    if (!iconMap.empty()) {
        const int maxVal = iconMap.rbegin()->first;

        auto it = iconMap.begin();
        for (int v = 0; v <= maxVal; ++v) {
            if (it != iconMap.end() && it->first == v) {
                items.append({ "", iconByIndex(it->second) });
                ++it;
            } else {
                items.append({ "", QPixmap() });
            }
        }
    }

    m_pictureEnums.emplace(idx, std::move(items));
}

std::vector<size_t> BackInfoCache::rebuildRefsFrom(const std::string& srcTable,
                                                   const RData& rdata,
                                                   RTablesDataManager* pRTDM)
{
    std::vector<size_t> updatedCols;
    if (!pRTDM || srcTable.empty())
        return updatedCols;

    // Быстрый доступ:
    // pluginIdx -> RCol*
    // pluginIdx -> позиция в rdata (pos)
    std::unordered_map<size_t, const RCol*> byPluginIdx;
    std::unordered_map<size_t, size_t>      posByPluginIdx;

    byPluginIdx.reserve(rdata.size());
    posByPluginIdx.reserve(rdata.size());

    size_t pos = 0;
    for (const RCol& rcol : rdata) {
        const size_t pluginIdx = static_cast<size_t>(rcol.getIndex());
        byPluginIdx.emplace(pluginIdx, &rcol);
        posByPluginIdx.emplace(pluginIdx, pos++);
    }

    const long nameIdx = pRTDM->column_index(srcTable, "name");

    auto findRCol = [&](size_t pluginIdx) -> const RCol* {
        auto it = byPluginIdx.find(pluginIdx);
        return (it != byPluginIdx.end()) ? it->second : nullptr;
    };

    auto getPos = [&](size_t pluginIdx) -> std::optional<size_t> {
        auto it = posByPluginIdx.find(pluginIdx);
        if (it == posByPluginIdx.end())
            return std::nullopt;
        return it->second;
    };

    auto rebuildNameref = [&](size_t pluginIdx, const RCol& rcol) -> bool {
        const auto parts = BackInfoCache::parseNamerefStr(rcol.getNameRef());
        if (!parts) return false;
        const long nameIdx = pRTDM->column_index(srcTable, "name");
        const std::string valueCol = (nameIdx > -1) ? "name" : parts->keyCol;
        m_nameref[pluginIdx] = buildIdNameMap(pRTDM, srcTable, parts->keyCol, valueCol);
        return true;
    };

    auto rebuildSuperenum = [&](size_t pluginIdx, const RCol& rcol) -> bool {
        const auto parts = BackInfoCache::parseSuperenumStr(rcol.getNameRef());
        if (!parts) return false;
        if (pRTDM->column_index(srcTable, parts->keyCol)   < 0 ||
            pRTDM->column_index(srcTable, parts->valueCol) < 0)
            return false;
        m_superenum[pluginIdx] = buildIdNameMap(pRTDM, srcTable,
                                                parts->keyCol, parts->valueCol);
        return true;
    };

    auto process = [&](const std::vector<size_t>& indices, auto&& rebuildFn) {
        for (size_t pluginIdx : indices) {
            const RCol* rcol = findRCol(pluginIdx);
            if (!rcol)
                continue;

            if (rebuildFn(pluginIdx, *rcol)) {
                if (auto posOpt = getPos(pluginIdx))
                    updatedCols.push_back(*posOpt); // позиционный индекс
            }
        }
    };

    if (auto it = m_namerefSources.find(srcTable); it != m_namerefSources.end())
        process(it->second, rebuildNameref);

    if (auto it = m_superenumSources.find(srcTable); it != m_superenumSources.end())
        process(it->second, rebuildSuperenum);

    return updatedCols;
}

BackInfoCache::RefMap BackInfoCache::buildIdNameMap(RTablesDataManager* pRTDM,
                                                    const std::string& srcTable,
                                                    const std::string& keyCol,
                                                    const std::string& valueCol)
{
    RefMap result;
    //result.emplace(0, "не задано");

    if (!pRTDM) return result;

    QDataBlock qdb;
    pRTDM->getDataBlock(srcTable, qdb, keyCol + "," + valueCol);

    for (int row = 0; row < qdb.RowsCount(); ++row) {
        const size_t key  = static_cast<size_t>(std::visit(ToLong(), qdb.Get(row, 0)));
        const std::string value = std::visit(ToString(), qdb.Get(row, 1));
        result.emplace(key, value);
    }

    return result;
}

std::optional<BackInfoCache::NamerefParts>
BackInfoCache::parseNamerefStr(const std::string& nameref)
{
    const size_t nopen  = nameref.find('[');
    const size_t nclose = nameref.find(']');
    if (nopen  == std::string::npos ||
        nclose == std::string::npos ||
        nclose <= nopen + 1)
        return std::nullopt;

    return NamerefParts{
        nameref.substr(0, nopen),
        nameref.substr(nopen + 1, nclose - nopen - 1)
    };
}

std::optional<BackInfoCache::SuperenumParts>
BackInfoCache::parseSuperenumStr(const std::string& nameref)
{
    const std::vector<std::string> parts{ split(nameref, '.') };
    if (parts.size() < 3)
        return std::nullopt;

    // Формат: "srcTable.valueCol.keyCol"
    return SuperenumParts{ parts[0], parts[2], parts[1] };
}

const QStringList* BackInfoCache::enumItems(size_t colIdx) const
{
    auto it = m_enum.find(colIdx);
    return it != m_enum.end() ? &it->second : nullptr;
}

const BackInfoCache::RefMap* BackInfoCache::superenumMap(size_t colIdx) const
{
    auto it = m_superenum.find(colIdx);
    return it != m_superenum.end() ? &it->second : nullptr;
}

const BackInfoCache::RefMap* BackInfoCache::namerefMap(size_t colIdx) const
{
    auto it = m_nameref.find(colIdx);
    return it != m_nameref.end() ? &it->second : nullptr;
}

const BackInfoCache::PictureList* BackInfoCache::pictureEnum(size_t pluginIdx) const
{
    auto it = m_pictureEnums.find(pluginIdx);
    return it != m_pictureEnums.end() ? &it->second : nullptr;
}

std::map<int, int> BackInfoCache::parseEnpicNameref(const std::string& nameref)
{
    std::map<int, int> result;
    QString s = QString::fromStdString(nameref).trimmed();

    auto parseInts = [](const QString& text) -> QList<int> {
        QList<int> values;
        for (const QString& token : text.split(',', Qt::SkipEmptyParts)) {
            bool ok = false;
            const int value = token.trimmed().toInt(&ok);
            if (ok)
                values << value;
        }
        return values;
    };

    // Если нет групп (нет ';') — просто плоский список иконок
    // "2,0,1,4,5" -> value=0 -> иконка 2, value=1 -> иконка 0, ...
    if (!s.contains(';')) {
        int fieldVal = 0;
        for (const int iconIdx : parseInts(s))
            result[fieldVal++] = iconIdx;
        return result;
    }

    // Группы: "(...),(...);(...)" — лишние скобки просто убираем
    QStringList groups;
    for (const QString& part : s.split(';', Qt::SkipEmptyParts))
        groups << QString(part).remove('(').remove(')').trimmed();

    // Группа 0: РисункиИстинаЛожь — [0]=иконка для value=1, [1]=иконка для value=0
    if (!groups.isEmpty()) {
        const QList<int> g = parseInts(groups[0]);
        if (!g.isEmpty())      result[1] = g[0];
        if (g.size() >= 2)     result[0] = g[1];
    }

    // Группа 1: РисункиИстина — значения начинаются с 2
    int nextVal = 2;
    if (groups.size() >= 2) {
        const QList<int> g = parseInts(groups[1]);
        for (const int idx : g)
            result[nextVal++] = idx;
    }

    // Группа 2: РисункиЛожь — значения продолжаются
    if (groups.size() >= 3) {
        const QList<int> g = parseInts(groups[2]);
        for (const int idx : g)
            result[nextVal++] = idx;
    }

    return result;
}

QPixmap BackInfoCache::iconByIndex(int idx)
{
    static const char* paths[] = {
        ":/images/grid/checkMark.png",
        ":/images/grid/exclamationMark.png",
        ":/images/grid/cross.png",
        ":/images/grid/blueFlag.png",
        ":/images/grid/leftBreaker.png",
        ":/images/grid/rightBreaker.png",
        ":/images/grid/redArrowLeft.png",
        ":/images/grid/redArrowRight.png",
        ":/images/grid/greenArrowLeft.png",
        ":/images/grid/greenArrowRight.png",
    };

    if (idx < 0 || idx >= static_cast<int>(std::size(paths))) {
        spdlog::warn("iconByIndex: unknown idx= {}", idx);
        return {};
    }

    QPixmap px(paths[idx]);
    if (px.isNull())
        spdlog::warn("iconByIndex: pixmap is NULL for idx= {}", idx);

    return px.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
