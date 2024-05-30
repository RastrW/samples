
#include <iostream>
#include <fstream>
#include "params.h"
#include "common.h"
#include "License2/json.hpp"


Params::Params(){
}

std::string Params::Get_on_start_load_file_rastr() const{
    return str_on_start_load_file_rastr_;
};

std::string Params::Get_on_start_load_file_forms() const{
    return str_on_start_load_file_forms_;
};

int Params::ReadJsonFile(std::filesystem::path path_2_json){
    try{
        plog(_err_code::norm, "read JSON file: [{}]\n", path_2_json.string());
        std::ifstream ifs(path_2_json);
        //nlohmann::json jfw; jfw[str_on_start_load_file_forms_] = "Общие";  std::ofstream o("c:/tmp/out.json"); o << std::setw(4) << jfw << std::endl;
        if(ifs.is_open()){
            nlohmann::json jf = nlohmann::json::parse(ifs);
            std::string sssdd = jf.dump(1, ' ', true);
            str_on_start_load_file_rastr_ = jf[on_start_load_file_rastr_];
            str_on_start_load_file_forms_ = jf[on_start_load_file_forms_];
            ifs.close();
        }else{
            plog(_err_code::fail, "can't open file [{}]\n", path_2_json.string());
        }
    }catch(const std::exception& ex){
        plog(_err_code::fail, "got exception [{}]\n", ex.what());
    }catch(...){
        plog(_err_code::fail, "got unknown exception. \n" );
    }
    return 1;
};
