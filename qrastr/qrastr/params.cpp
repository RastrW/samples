#include <iostream>
#include <fstream>
#include "params.h"
#include "common_qrastr.h"
#include "License2/json.hpp"

Params::Params(){
}
std::string Params::Get_on_start_load_file_rastr() const{
    return str_on_start_load_file_rastr_;
}
std::string Params::Get_on_start_load_file_forms() const{
    return str_on_start_load_file_forms_;
}
int Params::ReadJsonFile(std::filesystem::path path_2_json){
    try{
        //spdlog::info("read JSON file: [{}]", path_2_json.string());
        std::ifstream ifs(path_2_json);
        if(ifs.is_open()){
            nlohmann::json jf = nlohmann::json::parse(ifs);
            std::string sssdd = jf.dump(1, ' ', true);
            //spdlog::info("JSON : [{}]", sssdd);
            str_on_start_load_file_rastr_ = jf[on_start_load_file_rastr_];
            str_on_start_load_file_forms_ = jf[on_start_load_file_forms_];
            ifs.close();
        }else{
            //spdlog::error("Can't open file [{}]", path_2_json.string());
            return -3;
        }
    }catch(const std::exception& ex){
        //exclog(ex);
        return -1;
    }catch(...){
        //exclog();
        return -2;
    }
    return 1;
}
int Params::WriteJsonFile(std::filesystem::path path_2_json){
    try{
        std::ofstream ofs(path_2_json);
        if(ofs.is_open()){
            spdlog::warn("write JSON file: [{}]", path_2_json.string());
            /*
            nlohmann::json jf = nlohmann::json::parse(ifs);
            std::string sssdd = jf.dump(1, ' ', true);
            //spdlog::info("JSON : [{}]", sssdd);
            str_on_start_load_file_rastr_ = jf[on_start_load_file_rastr_];
            str_on_start_load_file_forms_ = jf[on_start_load_file_forms_];
            */
            ofs.close();
        }else{
            //spdlog::error("Can't open file [{}]", path_2_json.string());
            return -3;
        }
    }catch(const std::exception& ex){
        //exclog(ex);
        return -1;
    }catch(...){
        //exclog();
        return -2;
    }
    return 1;

}
