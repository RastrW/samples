#ifndef PARAMS_H
#define PARAMS_H
#pragma once

#include <QDir>
#include <singleton_dclp.hpp>
#include <filesystem>
namespace fs = std::filesystem;   

class CUIFormsCollection;

class Params
    : public SingletonDclp<Params>{
public:
    using _v_file_templates = std::vector<std::pair< std::string, std::string > >;
    using _v_forms = std::vector<std::string>;
    using _v_templates = std::vector<std::string>;
    using _v_template_exts = std::vector<std::pair< std::string, std::string > >;
    Params();
    virtual ~Params() = default;

    ///<Чтение/запись файла appsettings.json
    int readJsonFile (const fs::path& path_2_json);
    int writeJsonFile(const fs::path& path_2_json)const;
    static bool templ_sort_func(const std::pair<std::string,std::string> &p1,const std::pair<std::string,std::string> &p2);
    int readTemplates(const fs::path& path_dir_templates);
    int readForms    (const fs::path& path_form_load);
    int readFormsExists(const fs::path& path_dir_forms);
    const _v_forms& getFormsExists()const{
        return v_forms_exists_;
    }
    void setFileAppsettings(const fs::path& path_appsettings){
        path_appsettings_ = path_appsettings;
    }
    const fs::path& getFileAppsettings()const{
        return path_appsettings_;
    }
    void setDirData(const QDir& dir){
        dir_Data_   = dir;
        dir_SHABLON_ = dir_Data_;
        dir_SHABLON_.cd("SHABLON");
    }
    const QDir& getDirData( )const{
        return dir_Data_;
    }
    const QDir& getDirSHABLON( )const{
        return dir_SHABLON_;
    }
    const _v_file_templates& getStartLoadFileTemplates()const{
        return v_start_load_file_templates_;
    }
    void setStartLoadFileTemplates(const _v_file_templates& v_start_load_file_templates){
        v_start_load_file_templates_.clear();
        v_start_load_file_templates_.insert( v_start_load_file_templates_.begin(), v_start_load_file_templates.begin(), v_start_load_file_templates.end() );
    }
    void setStartLoadTemplates(const _v_templates& v_templates_in){
        v_start_load_templates_ = v_templates_in;
    }
    const _v_templates& getStartLoadTemplates()const{
        return v_start_load_templates_;
    }
    const _v_forms& getStartLoadForms()const{
        return v_start_load_forms_;
    }
    void setStartLoadForms(const _v_forms& v_start_load_file_forms){
        v_start_load_forms_.clear();
        v_start_load_forms_.insert( v_start_load_forms_.begin(), v_start_load_file_forms.begin(), v_start_load_file_forms.end() );
    }
    const _v_template_exts& getTemplateExts(){
        return v_template_exts_;
    }
private:
    QDir                  dir_Data_;
    QDir                  dir_SHABLON_;
    fs::path              path_appsettings_;
    _v_templates          v_start_load_templates_;
    _v_forms              v_start_load_forms_;
    _v_file_templates     v_start_load_file_templates_;
    _v_forms              v_forms_exists_;
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
    _v_template_exts      v_template_exts_;
public:
    static constexpr const char pch_stub_phrase_[]=               "not_set";
    static constexpr const char pch_dir_data_[]=                  "Data";
    static constexpr const char pch_org_qrastr_[]=                "QRastr";
    static constexpr const char pch_fname_appsettings[]=          "appsettings.json";
    static constexpr const char pch_json_start_[]=                "start";
    static constexpr const char pch_json_start_load_[]=           "load";
    static constexpr const char pch_json_start_load_file_[]=      "file";
    static constexpr const char pch_json_start_load_template_[]=  "template";
    static constexpr const char pch_json_start_forms_[]=          "forms";
    static constexpr const char pch_json_start_templates_[]=      "templates";
};

#endif // PARAMS_H
