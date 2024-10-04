#include <iostream>
#include <fstream>
#include "params.h"
#include "common_qrastr.h"
#include "License2/json.hpp"

Params::Params(){
}
int Params::readJsonFile(const std::filesystem::path& path_2_json){
    try{
        //spdlog::info("read JSON file: [{}]", path_2_json.string());
        v_file_templates_.clear();
        v_forms_.clear();
        v_templates_.clear();
        std::ifstream ifs(path_2_json);
        if(ifs.is_open()){
            const nlohmann::json jf = nlohmann::json::parse(ifs);
            //const std::string sssdd = jf.dump(1, ' ', true); //spdlog::info("JSON : [{}]", sssdd);
            const nlohmann::json j_start     = jf[ pch_json_start_ ];
            const nlohmann::json j_load      = j_start[pch_json_start_load_];
            for( const nlohmann::json& j_file_template : j_load ){
                std::string str_file     = j_file_template[pch_json_start_load_file_];
                std::string str_template = j_file_template[pch_json_start_load_template_];
                v_file_templates_.emplace_back(str_file, str_template);
            }
            const nlohmann::json j_forms = j_start[pch_json_start_forms_];
            for( const nlohmann::json& j_form : j_forms ){
                v_forms_.emplace_back(j_form);
            }
            const nlohmann::json j_templates = j_start[pch_json_start_templates_];
            for( const nlohmann::json& j_template : j_templates ){
                v_templates_.emplace_back(j_template);
            }
            ifs.close();
        }else{
            //spdlog::error("Can't open file [{}]", path_2_json.string());
            return -3;
        }
    }catch(const std::exception& ex){
        //exclog(ex);
        std::string str{ex.what()};
        return -1;
    }catch(...){
        //exclog();
        return -2;
    }
    return 1;
}
int Params::writeJsonFile(const std::filesystem::path& path_2_json)const {
    try{
        nlohmann::json jarr_load;
        for(const _v_file_templates::value_type& file_template : v_file_templates_){
            nlohmann::json j_file_temple;
            j_file_temple[pch_json_start_load_file_] = file_template.first;
            j_file_temple[pch_json_start_load_template_] = file_template.second;
            jarr_load.emplace_back(j_file_temple);
        }
        nlohmann::json jarr_forms;
        for(const _v_forms::value_type& form : v_forms_){
            jarr_forms.emplace_back(form);
        }
        nlohmann::json jarr_templates;
        for(const _v_templates::value_type& templ : v_templates_){
            jarr_templates.emplace_back(templ);
        }
        nlohmann::json j_start;
        j_start[pch_json_start_load_]      = jarr_load;
        j_start[pch_json_start_forms_]     = jarr_forms;
        j_start[pch_json_start_templates_] = jarr_templates;
        nlohmann::json j_file;
        j_file[pch_json_start_] = j_start;
        spdlog::warn("write JSON file: [{}]", path_2_json.string());
        std::ofstream ofs(path_2_json);
        if(ofs.is_open()){
            std::string  str_file = j_file.dump(1, ' ');
            //ofs << j_file;
            ofs << str_file;
            ofs.close();
        }else{
            spdlog::error("Can't open file [{}]", path_2_json.string());
            return -3;
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return -1;
    }catch(...){
        exclog();
        return -2;
    }
    return 1;

}
