#ifndef CONDFORMATJSON_H
#define CONDFORMATJSON_H
#pragma once

#include "CondFormat.h"
#include <fstream>
#include <filesystem>
#include <utils.h>
//#include "License2/json.hpp"
#include <astra/License2/json.hpp>

class CondFormatJson
{
public:
    //CondFormatJson();
    CondFormatJson(std::string _Table ,  std::vector<std::string>& _vcols, std::map<int, std::vector<CondFormat>>& _mcf)
    {
        path_2_json = std::filesystem::current_path() / j_fname_;
        m_Tname = _Table;
        m_cols = _vcols;
        m_MapcondFormatVector = _mcf;
    };

    void save_json();
    void save_json(nlohmann::json j);
    void append_json();
    void from_json();
    nlohmann::json to_json(std::string _tname = "");

    // Getter
    std::map<int, std::vector<CondFormat>> get_mcf()
    {
        return m_MapcondFormatVector;
    }

private:
    std::filesystem::path path_2_json;
    std::string m_Tname;
    std::vector<std::string> m_cols;
    std::map<int, std::vector<CondFormat>> m_MapcondFormatVector;

public:
    static constexpr const char j_fname_[] = "highlightsettings.json";
    static constexpr const char j_condformat_start_[] = "condformat";
    static constexpr const char j_cf_filter_[] = "filter";
    static constexpr const char j_cf_bgcolor_[] = "bgColor";
    static constexpr const char j_cf_fgcolor_[] = "fgColor";
};

#endif // CONDFORMATJSON_H
