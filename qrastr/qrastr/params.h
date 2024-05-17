#ifndef PARAMS_H
#define PARAMS_H

#include <filesystem>

constexpr char param_json_[] {  "on_start_load_file_rastr"};
static constexpr char on_start_load_file_rastr1 [] {"on_start_load_file_rastr"};

class Params
{
public:
    Params();
    virtual ~Params() = default;
    int ReadJsonFile(std::filesystem::path path_2_json);
    static constexpr char on_start_load_file_rastr []= "on_start_load_file_rastr";
    static constexpr char on_start_load_file_forms []= "on_start_load_file_forms";

};

#endif // PARAMS_H
