
#include <iostream>
#include <fstream>
#include "params.h"
#include "common.h"
#include "License2/json.hpp"

Params::Params(){

}

int Params::ReadJsonFile(std::filesystem::path path_2_json){
    try{
        std::ifstream ifs(path_2_json);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        std::string str_on_start_load_file_rastr = jf[on_start_load_file_rastr];
        std::string str_on_start_load_file_forms = jf[on_start_load_file_forms];

    }catch(const std::exception& ex){
        plog(_err_code::fail, "got exception [{}]\n", ex.what());
    }catch(...){
        plog(_err_code::fail, "got unknown exception. \n" );
    }

    return 1;
};
