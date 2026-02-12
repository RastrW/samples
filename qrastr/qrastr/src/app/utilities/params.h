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
    static bool templ_sort_func(const std::pair<std::string,std::string> &p1,
                                const std::pair<std::string,std::string> &p2);
    int readTemplates();
    int readForms    ();
    int readFormsExists();
    const _v_forms& getFormsExists()const{
        return m_forms_exists_;
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
        path_forms = dir_Data_.canonicalPath().toStdString()+"//form//";
    }

    const QDir& getDirData( )const{
        return dir_Data_;
    }
    const QDir& getDirSHABLON( )const{
        return dir_SHABLON_;
    }

    const std::filesystem::path& getPathForms( )const{
        return path_forms;
    }

    const _v_file_templates& getStartLoadFileTemplates()const{
        return m_start_load_file_templates_;
    }

    void setStartLoadFileTemplates(const _v_file_templates& start_load_file_templates){
        m_start_load_file_templates_.clear();
        m_start_load_file_templates_.insert(m_start_load_file_templates_.begin(),
                                            start_load_file_templates.begin(),
                                            start_load_file_templates.end() );
    }

    void setStartLoadTemplates(const _v_templates& templates_in){
        m_start_load_templates_ = templates_in;
    }

    const _v_templates& getStartLoadTemplates()const{
        return m_start_load_templates_;
    }

    const _v_forms& getStartLoadForms()const{
        return m_start_load_forms_;
    }

    void setStartLoadForms(const _v_forms& start_load_file_forms){
        m_start_load_forms_.clear();
        m_start_load_forms_.insert(m_start_load_forms_.begin(),
                                   start_load_file_forms.begin(),
                                   start_load_file_forms.end() );
    }
    const _v_template_exts& getTemplateExts(){
        return m_template_exts_;
    }
private:
    QDir                  dir_Data_;
    QDir                  dir_SHABLON_;
    fs::path              path_appsettings_;
    std::filesystem::path path_forms;
    /// Список загрузок по appsettings:
    // Файл для загрузки и соответствющие ему шаблоны
    _v_templates          m_start_load_templates_;
    // Список файлов из папки Data/form
    _v_forms              m_start_load_forms_;
    // Список файлов из папки Data/SHABLON
    _v_file_templates     m_start_load_file_templates_;
    /// Список всех возможных файлов
    // Список файлов в папке Data/form
    _v_forms              m_forms_exists_;
    // Список файлов в папке Data/SHABLON
    _v_template_exts      m_template_exts_;
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
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
