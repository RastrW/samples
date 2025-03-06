#include <iostream>
#include <fstream>
#include "params.h"
#include "common_qrastr.h"
#include "License2/json.hpp"
#include "UIForms.h"

Params::Params(){
}
int Params::readJsonFile(const fs::path& path_2_json){
    try{
        //spdlog::info("read JSON file: [{}]", path_2_json.string());
        v_start_load_file_templates_.clear();
        v_start_load_forms_.clear();
        v_start_load_templates_.clear();
        std::ifstream ifs(path_2_json);
        if(ifs.is_open()){
            const nlohmann::json jf = nlohmann::json::parse(ifs);
            //const std::string sssdd = jf.dump(1, ' ', true); //spdlog::info("JSON : [{}]", sssdd);
            const nlohmann::json j_start     = jf[ pch_json_start_ ];
            const nlohmann::json j_load      = j_start[pch_json_start_load_];
            for( const nlohmann::json& j_file_template : j_load ){
                std::string str_file     = j_file_template[pch_json_start_load_file_];
                std::string str_template = j_file_template[pch_json_start_load_template_];
                v_start_load_file_templates_.emplace_back(str_file, str_template);
            }
            const nlohmann::json j_forms = j_start[pch_json_start_forms_];
            for( const nlohmann::json& j_form : j_forms ){
                v_start_load_forms_.emplace_back(j_form);
            }
            const nlohmann::json j_templates = j_start[pch_json_start_templates_];
            for( const nlohmann::json& j_template : j_templates ){
                v_start_load_templates_.emplace_back(j_template);
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
int Params::writeJsonFile(const fs::path& path_2_json)const {
    try{
        nlohmann::json jarr_load;
        for(const _v_file_templates::value_type& file_template : v_start_load_file_templates_){
            nlohmann::json j_file_temple;
            j_file_temple[pch_json_start_load_file_] = file_template.first;
            j_file_temple[pch_json_start_load_template_] = file_template.second;
            jarr_load.emplace_back(j_file_temple);
        }
        nlohmann::json jarr_forms;
        for(const _v_forms::value_type& form : v_start_load_forms_){
            jarr_forms.emplace_back(form);
        }
        nlohmann::json jarr_templates;
        for(const _v_templates::value_type& templ : v_start_load_templates_){
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
bool Params::templ_sort_func(const std::pair<std::string,std::string>& p1,const std::pair<std::string,std::string>& p2)
{
    return (p1.first.length() < p2.first.length());
}
int Params::readTemplates(const fs::path& path_dir_templates){
    try{
        v_template_exts_.clear();
        for(const auto& entry : fs::directory_iterator(path_dir_templates)){
            fs::path path_template = entry.path();
            std::string str_templ_name = path_template.stem().u8string();
            std::string str_templ_ext  = path_template.extension().u8string();
            //spdlog::info("{}:{}", str_templ_name, str_templ_ext);
            v_template_exts_.emplace_back(std::make_pair(str_templ_name, str_templ_ext));
        }
        std::sort(v_template_exts_.begin(),v_template_exts_.end(),templ_sort_func);
    }catch(const std::exception& ex){
        exclog(ex);
        return -1;
    }catch(...){
        exclog();
        return -2;
    }
    return 1;
}
int Params::readForms(const fs::path& path_forms){
    try{
        /*
    #if(defined(_MSC_VER))
        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        //for (std::string &form : forms){
        for(const Params::_v_forms::value_type &form : Params::GetInstance()->getStartLoadForms()){
            std::filesystem::path path_file_form = stringutils::utf8_decode(form);
            path_form_load =  path_forms / path_file_form;
            qDebug() << "read form from file : " << path_form_load.wstring();
    #else
            path_forms_load = str_path_to_file_forms;
            qDebug() << "read form from file : " << path_forms_load.c_str();
    #endif

    */
        upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
        for(const Params::_v_forms::value_type &form : v_start_load_forms_){
            fs::path path_file_form  {path_forms};
            path_file_form /= stringutils::utf8_decode(form);
            CUIFormsCollection* CUIFormsCollection_ = new CUIFormsCollection ;
            if (path_file_form.extension() == ".fm")
                *CUIFormsCollection_ = CUIFormCollectionSerializerBinary(path_file_form).Deserialize();
            else
                *CUIFormsCollection_ = CUIFormCollectionSerializerJson(path_file_form).Deserialize();
            for(const  CUIForm& uiform : CUIFormsCollection_->Forms()){
                upCUIFormsCollection_->Forms().emplace_back(uiform);
            }
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
int Params::readFormsExists(const fs::path& path_dir_forms){
    try{
        v_forms_exists_.clear();
        for(const auto& entry : fs::directory_iterator(path_dir_forms)){
            fs::path path_form = entry.path();
            std::string str_form_name = path_form.filename().u8string();
            v_forms_exists_.emplace_back(str_form_name);
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



