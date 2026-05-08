#pragma once
#include <QString>
#include <QPixmap>
#include <map>
#include <optional>
#include "сolumnEditorInfo.h"
#include "table/tableIndexTypes.h"

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

    using RefMap = std::unordered_map<PluginIndex, std::string>;
    using PictureList = QList<PictureItem>;

    void rebuild(const RData& rdata, std::shared_ptr<ITableRepository>        tables);
    void clear();

    // Lookup helpers — возвращают nullptr / end() если нет данных для колонки.
    const QStringList*      enumItems(PluginIndex colIdx)     const;
    const RefMap*           superenumMap(PluginIndex colIdx)  const;
    const std::shared_ptr<ColumnEditorInfo::NameRefData>
        namerefData(PluginIndex colIdx) const;
    const PictureList*      pictureEnum(PluginIndex pluginIdx) const;

    /// Перестраивает только nameref/superenum-записи, чьи данные берутся из srcTable.
    /// Возвращает список позиционных индексов колонок, которые были обновлены.
    std::vector<PluginIndex> rebuildRefsFrom(const std::string&  srcTable,
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

    // plugin-индекс → список строк: ex. "БАЗА|Ген|Нагр|Ген+"
    std::unordered_map<PluginIndex, QStringList>
        m_enum;
    // plugin-индекс → { ключ → отображаемое имя }: ex. RefCol → node[na]
    std::unordered_map<PluginIndex,
                       std::shared_ptr<ColumnEditorInfo::NameRefData>>
        m_nameref;
    // plugin-индекс → { ключ → отображаемое имя }: ex. ti_prv.Name.Num
    std::unordered_map<PluginIndex, RefMap>
        m_superenum;
    // plugin-индекс → список картинок
    std::unordered_map<PluginIndex, PictureList>
        m_pictureEnums;

    std::unordered_map<std::string, std::vector<PluginIndex>>
        m_namerefSources;   // srcTable → {colIdx}
    std::unordered_map<std::string, std::vector<PluginIndex>>
        m_superenumSources;  // srcTable → {colIdx}
};
