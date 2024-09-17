#ifndef PARAMS_H
#define PARAMS_H

class Params{
public:
    Params();
    virtual ~Params() = default;
    int ReadJsonFile(std::filesystem::path path_2_json);
    std::string Get_on_start_load_file_rastr() const;
    std::string Get_on_start_load_file_forms() const;
    static constexpr char stub_phrase_              []= "not_set";
    static constexpr char on_start_load_file_rastr_ []= "on_start_load_file_rastr";
    static constexpr char on_start_load_file_forms_ []= "on_start_load_file_forms";
private:
    std::string str_on_start_load_file_rastr_ = stub_phrase_;
    std::string str_on_start_load_file_forms_ = stub_phrase_;
};

#endif // PARAMS_H
