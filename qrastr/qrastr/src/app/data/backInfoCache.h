#pragma once
#include <QString>
#include <QPixmap>
#include <map>
#include <optional>
#include "сolumnEditorInfo.h"
#include "table/tableIndexTypes.h"
#include "table/tableIndexHash.h"

class RData;
class ITableRepository;
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

    using RefMap = std::unordered_map<AstraIndex, std::string>;
    using PictureList = QList<PictureItem>;

    void rebuild(const RData& rdata, std::shared_ptr<ITableRepository>        tables);
    void clear();

    // Lookup helpers — возвращают nullptr / end() если нет данных для колонки.
    const QStringList*      enumItems(AstraIndex colIdx)     const;
    const RefMap*           superenumMap(AstraIndex colIdx)  const;
    const std::shared_ptr<ColumnEditorInfo::NameRefData>
        namerefData(AstraIndex colIdx) const;
    const PictureList*      pictureEnum(AstraIndex idx) const;

    /// Перестраивает только nameref/superenum-записи, чьи данные берутся из srcTable.
    /// Возвращает список позиционных индексов колонок, которые были обновлены.
    std::vector<ModelIndex> rebuildRefsFrom(const std::string&  srcTable,
                                          const RData&        rdata,
                                          std::shared_ptr<ITableRepository>        tables);
private:
    static std::map<int, int> parseEnpicNameref(const std::string& nameref);
    static QPixmap        iconByIndex(int idx);

    static RefMap
        buildIdNameMap(std::shared_ptr<ITableRepository> tables,
                       const std::string& srcTable,
                       const std::string& keyCol,
                       const std::string& valueCol);
    /// @brief Строит NameRefData из таблицы-источника.
    static std::shared_ptr<ColumnEditorInfo::NameRefData>
        buildNamerefData(const std::string& srcTable,
                         const std::string& keyCol,
                         std::shared_ptr<ITableRepository> tables);
    void loadEnum(const RCol& rcol);
    void loadSuperenum(const RCol& rcol, std::shared_ptr<ITableRepository>        tables);
    void loadNameref(const RCol& rcol, std::shared_ptr<ITableRepository>        tables);
    void loadEnpic(const RCol& rcol);

    ///Статические парсеры для ссылочных значений
    struct NamerefParts {
        std::string srcTable;
        std::string keyCol;
    };
    struct SuperenumParts {
        std::string srcTable;
        std::string keyCol;    // parts[2]
        std::string valueCol;  // parts[1]
    };
    static std::optional<NamerefParts>   parseNamerefStr
        (const std::string& nameref);
    static std::optional<SuperenumParts> parseSuperenumStr
        (const std::string& nameref);

    // astra-индекс → список строк: ex. "БАЗА|Ген|Нагр|Ген+"
    std::unordered_map<AstraIndex, QStringList>
        m_enum;
    // astra-индекс → { ключ → отображаемое имя }: ex. RefCol → node[na]
    std::unordered_map<AstraIndex,
                       std::shared_ptr<ColumnEditorInfo::NameRefData>>
        m_nameref;
    // astra-индекс → { ключ → отображаемое имя }: ex. ti_prv.Name.Num
    std::unordered_map<AstraIndex, RefMap>
        m_superenum;
    // astra-индекс → список картинок
    std::unordered_map<AstraIndex, PictureList>
        m_pictureEnums;

    std::unordered_map<std::string, std::vector<AstraIndex>>
        m_namerefSources;   // srcTable → {colIdx}
    std::unordered_map<std::string, std::vector<AstraIndex>>
        m_superenumSources;  // srcTable → {colIdx}
};
