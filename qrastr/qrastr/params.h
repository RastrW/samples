#ifndef PARAMS_H
#define PARAMS_H

#include <QDir>

#include <singleton_dclp.hpp>

class Params
    : public SingletonDclp<Params>{
public:
    Params();
    virtual ~Params() = default;
    int ReadJsonFile (std::filesystem::path path_2_json);
    int WriteJsonFile(std::filesystem::path path_2_json);
    std::string Get_on_start_load_file_rastr() const;
    std::string Get_on_start_load_file_forms() const;
    void setDirData(const QDir& qdir ){
        qdirData_ = qdir;
    }
    const QDir& getDirData( ){
        return qdirData_;
    }
    static constexpr char stub_phrase_              []= "not_set";
    static constexpr char on_start_load_file_rastr_ []= "on_start_load_file_rastr";
    static constexpr char on_start_load_file_forms_ []= "on_start_load_file_forms";
private:
    std::string str_on_start_load_file_rastr_ = stub_phrase_;
    std::string str_on_start_load_file_forms_ = stub_phrase_;
    QDir qdirData_;
};

#endif // PARAMS_H
