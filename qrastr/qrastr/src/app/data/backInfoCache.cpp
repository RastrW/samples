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
}

void BackInfoCache::rebuild(const RData& rdata, RTablesDataManager* pRTDM)
{
    clear();

    for (const RCol& rcol : rdata)
    {
        const size_t idx       = static_cast<size_t>(rcol.getIndex());
        const auto   propTT    = rcol.getComPropTT();
        const auto&  nameRef   = rcol.getNameRef();

        // ── ENUM ─────────────────────────────────────────────────────────────
        if (propTT == enComPropTT::COM_PR_ENUM) {
            QStringList list;
            for (const auto& val : split(nameRef, '|'))
                list.append(QString::fromStdString(val));
            m_enum.emplace(idx, std::move(list));
            continue;
        }

        // ── SUPERENUM ────────────────────────────────────────────────────────
        if (propTT == enComPropTT::COM_PR_SUPERENUM && !nameRef.empty()) {
            std::vector<std::string> parts{ split(nameRef, '.') };
            if (parts.size() > 2) {
                long indx1 = pRTDM->column_index(parts[0], parts[1]);
                long indx2 = pRTDM->column_index(parts[0], parts[2]);
                if (indx1 > -1 && indx2 > -1) {
                    QDataBlock qdb;
                    pRTDM->getDataBlock(parts[0], qdb, parts[2] + "," + parts[1]);

                    std::map<size_t, std::string> map;
                    map.emplace(0, "не задано");
                    for (int i = 0; i < qdb.RowsCount(); ++i) {
                        size_t      key = static_cast<size_t>(std::visit(ToLong(),   qdb.Get(i, 0)));
                        std::string val = std::visit(ToString(), qdb.Get(i, 1));
                        map.emplace(key, std::move(val));
                    }
                    m_superenum.emplace(idx, std::move(map));
                }
            }
            continue;
        }

        // ── NAMEREF ──────────────────────────────────────────────────────────
        if (propTT == enComPropTT::COM_PR_INT && !nameRef.empty()) {
            size_t nopen  = nameRef.find('[');
            size_t nclose = nameRef.find(']');
            if (nopen == std::string::npos || nclose == std::string::npos)
                continue;

            std::string table = nameRef.substr(0, nopen);
            std::string col   = nameRef.substr(nopen + 1, nclose - nopen - 1);

            long nameIndx   = pRTDM->column_index(table, "name");
            std::string cols = col + "," + (nameIndx > -1 ? std::string("name") : col);

            QDataBlock qdb;
            pRTDM->getDataBlock(table, qdb, cols);

            std::map<size_t, std::string> map;
            map.emplace(0, "не задано");
            for (int i = 0; i < qdb.RowsCount(); ++i) {
                size_t      key = static_cast<size_t>(std::visit(ToLong(), qdb.Get(i, 0)));
                std::string val = std::visit(ToString(), qdb.Get(i, 1));
                map.emplace(key, std::move(val));
            }
            m_nameref.emplace(idx, std::move(map));
            continue;
        }

        // ── ENPIC ────────────────────────────────────────────────────────────
        if (propTT == enComPropTT::COM_PR_ENPIC) {
            std::map<int, int> iconMap = parseEnpicNameref(nameRef);
            QList<PictureItem> items;

            if (!iconMap.empty()) {
                int maxVal = iconMap.rbegin()->first;

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
    }
}

const QStringList* BackInfoCache::enumItems(size_t colIdx) const
{
    auto it = m_enum.find(colIdx);
    return it != m_enum.end() ? &it->second : nullptr;
}

const std::map<size_t, std::string>* BackInfoCache::superenumMap(size_t colIdx) const
{
    auto it = m_superenum.find(colIdx);
    return it != m_superenum.end() ? &it->second : nullptr;
}

const std::map<size_t, std::string>* BackInfoCache::namerefMap(size_t colIdx) const
{
    auto it = m_nameref.find(colIdx);
    return it != m_nameref.end() ? &it->second : nullptr;
}

const QList<BackInfoCache::PictureItem>* BackInfoCache::pictureEnum(size_t pluginIdx) const
{
    auto it = m_pictureEnums.find(pluginIdx);
    return it != m_pictureEnums.end() ? &it->second : nullptr;
}

std::map<int, int> BackInfoCache::parseEnpicNameref(const std::string& nameref)
{
    std::map<int, int> result;
    QString s = QString::fromStdString(nameref).trimmed();
    // Если нет групп (нет ';') — просто плоский список иконок
    // "2,0,1,4,5" -> value=0 -> иконка 2, value=1 -> иконка 0, ...
    if (!s.contains(';')) {
        int fieldVal = 0;
        for (const QString& t : s.split(',', Qt::SkipEmptyParts)) {
            bool ok = false;
            int iconIdx = t.trimmed().toInt(&ok);
            if (ok) result[fieldVal++] = iconIdx;
        }
        return result;
    }
    // Разбиваем на группы по ';', убираем скобки
    QStringList groups;
    for (const QString& part : s.split(';', Qt::SkipEmptyParts))
        groups << QString(part).remove('(').remove(')').trimmed();

    auto parseGroup = [](const QString& g) -> QList<int> {
        QList<int> res;
        for (const QString& t : g.split(',', Qt::SkipEmptyParts)) {
            bool ok = false;
            int v = t.trimmed().toInt(&ok);
            if (ok) res << v;
        }
        return res;
    };
    // Группа 0: РисункиИстинаЛожь — [0]=иконка для value=1, [1]=иконка для value=0
    if (!groups.isEmpty()) {
        QList<int> g = parseGroup(groups[0]);
        if (g.size() >= 1) result[1] = g[0];
        if (g.size() >= 2) result[0] = g[1];
    }
    // Группа 1: РисункиИстина — значения начинаются с 2
    int nextVal = 2;
    if (groups.size() >= 2)
        for (int idx : parseGroup(groups[1]))
            result[nextVal++] = idx;
    // Группа 2: РисункиЛожь — значения продолжаются
    if (groups.size() >= 3)
        for (int idx : parseGroup(groups[2]))
            result[nextVal++] = idx;

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
