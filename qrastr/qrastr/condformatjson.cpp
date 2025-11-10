#include "condformatjson.h"

void CondFormatJson::save_json(nlohmann::json j)
{
    std::string jstr = j.dump();
    std::ofstream ofs(path_2_json);
    if (ofs.is_open())
    {
        std::string  str_file = j.dump(1, ' ');
        ofs << str_file;
        ofs.close();
    }
}
void CondFormatJson::save_json()
{
    if (std::filesystem::exists(path_2_json))
    {
        append_json();
    }
    else
    {
        nlohmann::json js = to_json();
        save_json(js);
    }

}
nlohmann::json CondFormatJson::to_json(std::string _tname)
{
    nlohmann::json j;
    nlohmann::json j_Tables;
    nlohmann::json j_Table;

    for (auto &[key, val] : m_MapcondFormatVector)
    {
        if (val.empty())
            continue;
        nlohmann::json j_col_condformats;
        std::string col_name = m_cols.at(key);
        for (auto& cf : val)
        {
            nlohmann::json j_col_condformat;
            j_col_condformat[j_cf_filter_] = cf.filter().toStdString();
            j_col_condformat[j_cf_bgcolor_] = cf.backgroundColor().name().toStdString();
            j_col_condformat[j_cf_fgcolor_] = cf.foregroundColor().name().toStdString();
            j_col_condformats.emplace_back(j_col_condformat);
        }
        j_Table[col_name].emplace_back(j_col_condformats);
    }
    //j_Tables[m_Tname].emplace_back(j_Table);
    j_Tables[m_Tname] = j_Table;
    if (!_tname.empty())
        return j_Table;

    //j[j_condformat_start_].emplace_back(j_Tables);
    j[j_condformat_start_] = j_Tables;

    return j;
}
void CondFormatJson::append_json()
{
    std::ifstream ifs(path_2_json);
    nlohmann::json jf = nlohmann::json::parse(ifs);
    nlohmann::json j_Tables = jf[j_condformat_start_];

    std::cout << j_Tables.type_name() << std::endl;
    std::cout << j_Tables << std::endl;
    //for (auto it = j_Tables.begin(); it != j_Tables.end(); ++it)
    //	std::cout << it.key() << " : " << it.value() << std::endl;

    nlohmann::json j_Table = to_json(m_Tname);
    //j_Tables.push_back(j_Table);
    j_Tables[m_Tname] = j_Table;
    jf[j_condformat_start_] = j_Tables;

    save_json(jf);
}
void CondFormatJson::from_json()
{
    if (!std::filesystem::exists( path_2_json))
        return;

    m_MapcondFormatVector.clear();
    std::ifstream ifs(path_2_json);
    const nlohmann::json jf = nlohmann::json::parse(ifs);
    const nlohmann::json j_Tables = jf[j_condformat_start_];
    bool bexist = j_Tables.contains(m_Tname);
    if (!j_Tables.contains(m_Tname))
        return;

    const nlohmann::json j_Table = j_Tables[m_Tname];

    for (auto it = j_Table.begin(); it != j_Table.end(); ++it)
    {
        //std::cout << it.key() << " : " << it.value() << std::endl;
        int index = find_index(m_cols, it.key());
        std::vector<CondFormat> vcf;
        m_MapcondFormatVector.insert(make_pair(index, vcf));
        const nlohmann::json j_Table_col = j_Table[it.key()];
        for (const nlohmann::json& j_col_condformats : j_Table_col)
        {
            for (const nlohmann::json& j_col_condformat : j_col_condformats)
            {
                std::string str_filter = j_col_condformat[j_cf_filter_];
                std::string str_bgc = j_col_condformat[j_cf_bgcolor_];
                std::string str_fgc = j_col_condformat[j_cf_fgcolor_];
#if(defined(_MSC_VER))
                vcf.emplace_back(   QString(str_filter.c_str()),
                                    QColor::fromString(str_fgc.c_str()),
                                    QColor::fromString(str_bgc.c_str()),
                                    QFont(),
                                    CondFormat::Alignment::AlignLeft,
                                    "UTF-8");
#else
                vcf.emplace_back(   QString(str_filter.c_str()),
                                    QColor(str_fgc.c_str()), // nix: in qt5 no QColor::fromString()
                                    QColor(str_bgc.c_str()), // nix: in qt5 no QColor::fromString()
                                    QFont(),
                                    CondFormat::Alignment::AlignLeft,
                                    "UTF-8");
#endif
            }
            m_MapcondFormatVector.at(index) = vcf;
        }
    }
}

