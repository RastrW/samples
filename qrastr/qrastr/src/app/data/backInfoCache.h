#pragma once
#include <QString>
#include <QPixmap>
#include <map>

class RData;
class RTablesDataManager;
class RCol;

///@class Справочные данные для отображения ячеек:
///   ENUM / SUPERENUM / NAMEREF / ENPIC
class BackInfoCache
{
public:
    struct PictureItem {
        QString label;
        QPixmap image;
    };

    using RefMap      = std::unordered_map<size_t, std::string>;
    using PictureList = QList<PictureItem>;

    void rebuild(const RData& rdata, RTablesDataManager* pRTDM);
    void clear();

    // Lookup helpers — возвращают nullptr / end() если нет данных для колонки.
    const QStringList*      enumItems(size_t colIdx)     const;
    const RefMap*           superenumMap(size_t colIdx)  const;
    const RefMap*           namerefMap(size_t colIdx)    const;
    const PictureList*      pictureEnum(size_t pluginIdx) const;

    /// Перестраивает только nameref/superenum-записи, чьи данные берутся из srcTable.
    /// Возвращает список позиционных индексов колонок, которые были обновлены.
    std::vector<size_t> rebuildRefsFrom(const std::string&  srcTable,
                                        const RData&        rdata,
                                        RTablesDataManager* pRTDM);
private:
    static std::map<int, int> parseEnpicNameref(const std::string& nameref);
    static QPixmap        iconByIndex(int idx);

    static RefMap  buildIdNameMap(RTablesDataManager* pRTDM,
                                 const std::string& srcTable,
                                 const std::string& keyCol,
                                 const std::string& valueCol);
    void loadEnum(const RCol& rcol);
    void loadSuperenum(const RCol& rcol, RTablesDataManager* pRTDM);
    void loadNameref(const RCol& rcol, RTablesDataManager* pRTDM);
    void loadEnpic(const RCol& rcol);

    // plugin-индекс → список строк: ex. "БАЗА|Ген|Нагр|Ген+"
    std::unordered_map<size_t, QStringList>            m_enum;
    // plugin-индекс → { ключ → отображаемое имя }: ex. RefCol → node[na]
    std::unordered_map<size_t, RefMap>                 m_nameref;
    // plugin-индекс → { ключ → отображаемое имя }: ex. ti_prv.Name.Num
    std::unordered_map<size_t, RefMap>                 m_superenum;
    // plugin-индекс → список картинок
    std::unordered_map<size_t, PictureList>            m_pictureEnums;

    std::unordered_map<std::string, std::vector<size_t>> m_namerefSources;   // srcTable → {colIdx}
    std::unordered_map<std::string, std::vector<size_t>> m_superenumSources;  // srcTable → {colIdx}
};
