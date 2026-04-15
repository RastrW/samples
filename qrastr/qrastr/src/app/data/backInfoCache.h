#pragma once
#include <QString>
#include <QPixmap>
#include <map>

class RData;
class RTablesDataManager;

///@class Справочные данные для отображения ячеек:
///   ENUM / SUPERENUM / NAMEREF / ENPIC
class BackInfoCache
{
public:
    struct PictureItem {
        QString label;
        QPixmap image;
    };

    void rebuild(const RData& rdata, RTablesDataManager* pRTDM);
    void clear();

    // Lookup helpers — возвращают nullptr / end() если нет данных для колонки.
    const QStringList*                            enumItems(size_t colIdx)     const;
    const std::map<size_t, std::string>*          superenumMap(size_t colIdx)  const;
    const std::map<size_t, std::string>*          namerefMap(size_t colIdx)    const;
    const QList<PictureItem>*                     pictureEnum(size_t pluginIdx) const;

    /// Перестраивает только nameref/superenum-записи, чьи данные берутся из srcTable.
    /// Возвращает список позиционных индексов колонок, которые были обновлены.
    std::vector<size_t> rebuildRefsFrom(const std::string&  srcTable,
                                        const RData&        rdata,
                                        RTablesDataManager* pRTDM);
private:
    static std::map<int, int> parseEnpicNameref(const std::string& nameref);
    static QPixmap        iconByIndex(int idx);

    // индекс колонки → список строк: ex. "БАЗА|Ген|Нагр|Ген+"
    std::map<size_t, QStringList>                         m_enum;
    // колонка → { код → отображаемое имя }: ex. RefCol → node[na]
    std::map<size_t, std::map<size_t, std::string>>       m_nameref;
    // колонка → { код → отображаемое имя }: ex. ti_prv.Name.Num
    std::map<size_t, std::map<size_t, std::string>>       m_superenum;
    // plugin-индекс → список картинок
    std::map<size_t, QList<PictureItem>>                  m_pictureEnums;

    std::map<std::string, std::vector<size_t>> m_namerefSources;   // srcTable → {colIdx}
    std::map<std::string, std::vector<size_t>> m_superenumSources; // srcTable → {colIdx}
};
