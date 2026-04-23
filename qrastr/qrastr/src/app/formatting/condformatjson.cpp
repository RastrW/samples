#include "condformatjson.h"
#include <fstream>
#include "condFormat.h"

#include <utils.h>


static int findColIndex(const std::vector<std::string>& cols, const std::string& name)
{
    auto it = std::find(cols.begin(), cols.end(), name);
    return (it != cols.end()) ? static_cast<int>(it - cols.begin()) : -1;
}

nlohmann::json CondFormatJson::tableToJson(
    const std::vector<std::string>& cols,
    const std::unordered_map<int, std::vector<CondFormat>>& formats)
{
    nlohmann::json j;
    for (auto& [key, val] : formats) {
        if (val.empty()) continue;
        if (key < 0 || static_cast<size_t>(key) >= cols.size()) continue;

        nlohmann::json colFormats = nlohmann::json::array();
        for (const auto& cf : val) {
            colFormats.push_back({
                { kFilter,  cf.filter().toStdString() },
                { kBgColor, cf.backgroundColor().name().toStdString() },
                { kFgColor, cf.foregroundColor().name().toStdString() }
            });
        }
        j[cols.at(key)] = colFormats;
    }
    return j;
}

void CondFormatJson::save(
    const std::string& tableName,
    const std::vector<std::string>& cols,
    const std::unordered_map<int, std::vector<CondFormat>>& formats)
{
    const QString path = jsonPath();
    // Загружаем существующий файл (если есть), чтобы не затереть другие таблицы.
    nlohmann::json root;
    QFile fileIn(path);
    if (fileIn.exists() && fileIn.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray raw = fileIn.readAll();
        root = nlohmann::json::parse(
            raw.constData(), raw.constData() + raw.size(),
            nullptr, /*exceptions=*/false);
        if (root.is_discarded()) root = nlohmann::json::object();
    }
    // Перезаписываем только секцию нашей таблицы.
    root[kRoot][tableName] = tableToJson(cols, formats);

    QFile fileOut(path);
    if (!fileOut.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    const std::string dump = root.dump(1, ' ');
    fileOut.write(dump.data(), static_cast<qint64>(dump.size()));
}

std::unordered_map<int, std::vector<CondFormat>>
CondFormatJson::load(const std::string& tableName,
                     const std::vector<std::string>& cols)
{
    std::unordered_map<int, std::vector<CondFormat>> result;

    const QString path = jsonPath();
    if (!QFileInfo::exists(path)) return result;        // QFileInfo::exists

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return result;
    const QByteArray raw = file.readAll();              // QFile::readAll

    const auto root = nlohmann::json::parse(
        raw.constData(), raw.constData() + raw.size(),
        nullptr, /*exceptions=*/false);
    if (root.is_discarded()) return result;

    if (!root.contains(kRoot) || !root[kRoot].contains(tableName))
        return result;

    const auto& jTable = root[kRoot][tableName];
    for (auto it = jTable.begin(); it != jTable.end(); ++it) {
        int idx = findColIndex(cols, it.key());
        if (idx < 0) continue; // колонка переименована или удалена — пропускаем

        std::vector<CondFormat> vcf;
        for (const auto& jcf : it.value()) {
            std::string filter = jcf.value(kFilter, "");
            std::string bgc    = jcf.value(kBgColor, "#ffffff");
            std::string fgc    = jcf.value(kFgColor, "#000000");

#ifdef _MSC_VER
            vcf.emplace_back(
                QString::fromStdString(filter),
                QColor::fromString(fgc.c_str()),
                QColor::fromString(bgc.c_str()),
                QFont(), CondFormat::AlignLeft, "UTF-8");
#else
            vcf.emplace_back(
                QString::fromStdString(filter),
                QColor(fgc.c_str()),
                QColor(bgc.c_str()),
                QFont(), CondFormat::AlignLeft, "UTF-8");
#endif
        }
        if (!vcf.empty())
            result[idx] = std::move(vcf);
    }
    return result;
}
