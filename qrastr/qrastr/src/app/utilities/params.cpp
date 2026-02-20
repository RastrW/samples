#include "params.h"
#include <fstream>
#include "common_qrastr.h"
#include <astra_headers/json.hpp>
#include "astra_headers/UIForms.h"

Params::Params(){
}

bool Params::readJsonFile(const fs::path& path_2_json){
    // Значения по умолчанию
    static const _v_forms default_forms = {
        "poisk.fm",
        "Анцапфы.fm",
        "Общие.fm",
        "ти.fm",
        "трансформаторы.fm"
    };
    static const _v_templates default_templates = {
        "context.form",
        "режим.rg2",
        "анцапфы.anc",
        "сечения.sch",
        "траектория утяжеления.ut2"
    };

    try{
        m_start_load_file_templates_.clear();
        m_start_load_forms_.clear();
        m_start_load_templates_.clear();

        std::ifstream ifs(path_2_json);
        if(!ifs.is_open()){
            // Файл не существует — используем значения по умолчанию, это не ошибка
            spdlog::warn("appsettings.json not found, using defaults: [{}]",
                         path_2_json.string());
            m_start_load_forms_     = default_forms;
            m_start_load_templates_ = default_templates;
            return true;
        }

        const nlohmann::json jf = nlohmann::json::parse(ifs);
        ifs.close();

        if(!jf.contains(pch_json_start_) || !jf[pch_json_start_].is_object()){
            spdlog::warn("appsettings.json: missing 'start' section, using defaults");
            m_start_load_forms_     = default_forms;
            m_start_load_templates_ = default_templates;
            return true;
        }

        const nlohmann::json& j_start = jf[pch_json_start_];

        // forms — при отсутствии используем defaults
        if(j_start.contains(pch_json_start_forms_) &&
            j_start[pch_json_start_forms_].is_array()){
            for(const nlohmann::json& j_form : j_start[pch_json_start_forms_]){
                m_start_load_forms_.emplace_back(j_form.get<std::string>());
            }
        }else{
            spdlog::warn("appsettings.json: 'forms' missing or invalid, using defaults");
            m_start_load_forms_ = default_forms;
        }

        // templates — при отсутствии используем defaults
        if(j_start.contains(pch_json_start_templates_) &&
            j_start[pch_json_start_templates_].is_array()){
            for(const nlohmann::json& j_template : j_start[pch_json_start_templates_]){
                m_start_load_templates_.emplace_back(j_template.get<std::string>());
            }
        }else{
            spdlog::warn("appsettings.json: 'templates' missing or invalid, using defaults");
            m_start_load_templates_ = default_templates;
        }

        // load — просто пропускаем если нет
        if(j_start.contains(pch_json_start_load_) &&
            j_start[pch_json_start_load_].is_array()){
            for(const nlohmann::json& j_file_template : j_start[pch_json_start_load_]){
                if(!j_file_template.contains(pch_json_start_load_file_) ||
                    !j_file_template.contains(pch_json_start_load_template_)){
                    spdlog::warn("appsettings.json: skipping malformed 'load' entry");
                    continue;
                }
                std::string str_file     = j_file_template[pch_json_start_load_file_].get<std::string>();
                std::string str_template = j_file_template[pch_json_start_load_template_].get<std::string>();
                m_start_load_file_templates_.emplace_back(str_file, str_template);
            }
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool Params::writeJsonFile(const fs::path& path_2_json)const {
    ///@note Здесь уже нельзя использовать spdlog
    try{
        nlohmann::json j_start;
        // forms — всегда пишем
        nlohmann::json jarr_forms = nlohmann::json::array();
        for(const _v_forms::value_type& form : m_start_load_forms_){
            jarr_forms.emplace_back(form);
        }
        j_start[pch_json_start_forms_] = jarr_forms;

        // templates — всегда пишем
        nlohmann::json jarr_templates = nlohmann::json::array();
        for(const _v_templates::value_type& templ : m_start_load_templates_){
            jarr_templates.emplace_back(templ);
        }
        j_start[pch_json_start_templates_] = jarr_templates;

        // load — только если пользователь что-то загружал
        if(!m_start_load_file_templates_.empty()){
            nlohmann::json jarr_load = nlohmann::json::array();
            for(const _v_file_templates::value_type& file_template : m_start_load_file_templates_){
                nlohmann::json j_entry;
                j_entry[pch_json_start_load_file_]     = file_template.first;
                j_entry[pch_json_start_load_template_] = file_template.second;
                jarr_load.emplace_back(j_entry);
            }
            j_start[pch_json_start_load_] = jarr_load;
        }

        nlohmann::json j_file;
        j_file[pch_json_start_] = j_start;

        qInfo() << "write JSON file: [" << path_2_json.string() << "]\n";
        std::ofstream ofs(path_2_json);
        if(ofs.is_open()){
            ofs << j_file.dump(1, ' ');
            ofs.close();
        }else{
            qCritical() <<"Can't open file for write: [{}]", path_2_json.string();
            return false;
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool Params::templ_sort_func(const std::pair<std::string,std::string>& p1,
                             const std::pair<std::string,std::string>& p2)
{
    return (p1.first.length() < p2.first.length());
}

bool Params::readTemplates(){
    try{
        const fs::path& path_dir_templates
            {getDirSHABLON().absolutePath().toStdString()};
        m_template_exts_.clear();
        for(const auto& entry : fs::directory_iterator(path_dir_templates)){
            fs::path path_template = entry.path();
            std::string str_templ_name = path_template.stem().u8string();
            std::string str_templ_ext  = path_template.extension().u8string();
            m_template_exts_.emplace_back
                (std::make_pair(str_templ_name, str_templ_ext));
        }
        std::sort(m_template_exts_.begin(),m_template_exts_.end(),templ_sort_func);
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool Params::readForms(){
    try{
        upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
        for(const Params::_v_forms::value_type &form : m_start_load_forms_){
            fs::path path_file_form  {path_forms};
            path_file_form /= stringutils::utf8_decode(form);

            auto t_сollection = std::make_unique<CUIFormsCollection>();
            if (path_file_form.extension() == ".fm")
                *t_сollection = CUIFormCollectionSerializerBinary
                                       (path_file_form).Deserialize();
            else
                *t_сollection = CUIFormCollectionSerializerJson
                                       (path_file_form).Deserialize();
            for(const  CUIForm& uiform : t_сollection->Forms()){
                upCUIFormsCollection_->Forms().emplace_back(uiform);
            }
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}

bool Params::readFormsExists(){
    try{
        m_forms_exists_.clear();
        for(const auto& entry : fs::directory_iterator(path_forms)){
            fs::path path_form = entry.path();
            std::string str_form_name = path_form.filename().u8string();
            m_forms_exists_.emplace_back(str_form_name);
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return false;
    }catch(...){
        exclog();
        return false;
    }
    return true;
}


void Params::addStartLoadFileTemplate(const std::string& file,
                                      const std::string& templ){
    // Избегаем дублирования
    auto it = std::find_if(m_start_load_file_templates_.begin(),
                           m_start_load_file_templates_.end(),
                           [&file](const _v_file_templates::value_type& p){
                               return p.first == file;
                           });
    if(it != m_start_load_file_templates_.end()){
        it->second = templ; // Обновляем шаблон если файл уже есть
    }else{
        m_start_load_file_templates_.emplace_back(file, templ);
    }
}
