#pragma once

#include <QDir>
#include <singleton_dclp.hpp>

class CUIFormsCollection;

class RastrParameters
    : public SingletonDclp<RastrParameters>{
public:
    using _v_file_templates  = std::vector<std::pair<std::string, std::string>>;
    using _v_forms           = std::vector<std::string>;
    using _v_templates       = std::vector<std::string>;
    using _v_template_exts   = std::vector<std::pair<std::string, std::string>>;

    RastrParameters();
    virtual ~RastrParameters() = default;

    ///<Чтение/запись файла appsettings.json
    bool readJsonFile (const QString& path);
    bool writeJsonFile(const QString& path) const;

    bool readTemplates();
    bool readForms();
    bool readFormsExists();

    const _v_forms& getFormsExists() const { return m_forms_exists_; }

    void setFileAppsettings(const QString& path) { path_appsettings_ = path; }
    const QString& getFileAppsettings() const     { return path_appsettings_; }

    void setDirData(const QDir& dir) {
        dir_Data_    = dir;
        dir_SHABLON_ = dir_Data_;
        dir_SHABLON_.cd("SHABLON");

        dir_forms_   = dir_Data_;
        dir_forms_.cd("form");
    }

    const QDir& getDirData()    const { return dir_Data_;    }
    const QDir& getDirSHABLON() const { return dir_SHABLON_; }
    const QDir& getDirForms()   const { return dir_forms_;   }

    std::string getLogDirPath() const {
        return dir_Data_.absolutePath().toStdString();
    }

    const _v_file_templates& getStartLoadFileTemplates() const {
        return m_start_load_file_templates_;
    }
    void setStartLoadFileTemplates(const _v_file_templates& v) {
        m_start_load_file_templates_ = v;
    }

    void setStartLoadTemplates(const _v_templates& v) { m_start_load_templates_ = v; }
    const _v_templates& getStartLoadTemplates() const { return m_start_load_templates_; }

    const _v_forms& getStartLoadForms() const { return m_start_load_forms_; }
    void setStartLoadForms(const _v_forms& v)  { m_start_load_forms_ = v;   }

    const _v_template_exts& getTemplateExts() { return m_template_exts_; }

private:
    QDir    dir_Data_;
    QDir    dir_SHABLON_;
    QDir    dir_forms_;
    QString path_appsettings_;
    /// Список загрузок по appsettings:
    // Файл для загрузки и соответствющие ему шаблоны
    _v_templates      m_start_load_templates_;
    // Список файлов из папки Data/form (только .fm формат)
    ///@note Использование json форм приводит к падению при их загрузке
    _v_forms          m_start_load_forms_;
    // Список файлов из папки Data/SHABLON
    _v_file_templates m_start_load_file_templates_;
    /// Список всех возможных файлов
    // Список файлов в папке Data/form (только .fm формат)
    _v_forms          m_forms_exists_;
    // Список файлов в папке Data/SHABLON
    _v_template_exts  m_template_exts_;

    static bool templ_sort_func(const std::pair<std::string,std::string>& p1,
                                const std::pair<std::string,std::string>& p2);

public:
    static constexpr const char pch_dir_data_[]                 = "Data";
    static constexpr const char pch_fname_appsettings[]         = "appsettings.json";
    static constexpr const char pch_json_start_[]               = "start";
    static constexpr const char pch_json_start_load_[]          = "load";
    static constexpr const char pch_json_start_load_file_[]     = "file";
    static constexpr const char pch_json_start_load_template_[] = "template";
    static constexpr const char pch_json_start_forms_[]         = "forms";
    static constexpr const char pch_json_start_templates_[]     = "templates";
};
