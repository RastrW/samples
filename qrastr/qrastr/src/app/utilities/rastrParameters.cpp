#include "rastrParameters.h"
#include <fstream>
#include "common_qrastr.h"
#include "json.hpp"
#include "UIForms.h"
#include <QDirIterator>

RastrParameters::RastrParameters() = default;

bool RastrParameters::readJsonFile(const QString& path){
    // Значения по умолчанию
    static const _v_forms default_forms = {
        "poisk.fm", "Анцапфы.fm", "Общие.fm", "ти.fm", "трансформаторы.fm"
    };
    static const _v_templates default_templates = {
        "context.form", "режим.rg2", "анцапфы.anc",
        "сечения.sch",  "траектория утяжеления.ut2"
    };

    try {
        m_start_load_file_templates_.clear();
        m_start_load_forms_.clear();
        m_start_load_templates_.clear();

        QFile file(path);
        if (!file.exists()) {
            // Файл не существует — используем значения по умолчанию, это не ошибка
            spdlog::warn("appsettings.json not found, using defaults: [{}]",
                         path.toStdString());
            m_start_load_forms_     = default_forms;
            m_start_load_templates_ = default_templates;
            return true;
        }

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            spdlog::warn("appsettings.json not readable (permissions?): [{}]",
                         path.toStdString());
            m_start_load_forms_     = default_forms;
            m_start_load_templates_ = default_templates;
            return true;  // не фатально — работаем с дефолтами
        }

        const QByteArray raw = file.readAll();
        file.close();

        const nlohmann::json jf = nlohmann::json::parse(
            raw.constData(), raw.constData() + raw.size());

        if (!jf.contains(pch_json_start_) || !jf[pch_json_start_].is_object()) {
            spdlog::warn("appsettings.json: missing 'start' section, using defaults");
            m_start_load_forms_     = default_forms;
            m_start_load_templates_ = default_templates;
            return true;
        }

        const nlohmann::json& j_start = jf[pch_json_start_];
        // forms — при отсутствии используем defaults
        if (j_start.contains(pch_json_start_forms_) &&
            j_start[pch_json_start_forms_].is_array()) {
            for (const auto& j_form : j_start[pch_json_start_forms_]) {
                m_start_load_forms_.emplace_back(j_form.get<std::string>());
            }
        } else {
            spdlog::warn("appsettings.json: 'forms' missing or invalid, using defaults");
            m_start_load_forms_ = default_forms;
        }
        // templates — при отсутствии используем defaults
        if (j_start.contains(pch_json_start_templates_) &&
            j_start[pch_json_start_templates_].is_array()) {
            for (const auto& j_tmpl : j_start[pch_json_start_templates_]) {
                m_start_load_templates_.emplace_back(j_tmpl.get<std::string>());
            }
        } else {
            spdlog::warn("appsettings.json: 'templates' missing or invalid, using defaults");
            m_start_load_templates_ = default_templates;
        }
        // load — просто пропускаем если нет
        if (j_start.contains(pch_json_start_load_) &&
            j_start[pch_json_start_load_].is_array()) {
            for (const auto& j_entry : j_start[pch_json_start_load_]) {
                if (!j_entry.contains(pch_json_start_load_file_) ||
                    !j_entry.contains(pch_json_start_load_template_)) {
                    spdlog::warn("appsettings.json: skipping malformed 'load' entry");
                    continue;
                }
                m_start_load_file_templates_.emplace_back(
                    j_entry[pch_json_start_load_file_].get<std::string>(),
                    j_entry[pch_json_start_load_template_].get<std::string>());
            }
        }

    } catch (const std::exception& ex) {
        exclog(ex);
        return false;
    } catch (...) {
        exclog();
        return false;
    }
    return true;
}

bool RastrParameters::writeJsonFile(const QString& path) const {
    try {
        nlohmann::json j_start;
        // forms — всегда пишем
        nlohmann::json jarr_forms = nlohmann::json::array();
        for (const auto& form : m_start_load_forms_)
            jarr_forms.emplace_back(form);
        j_start[pch_json_start_forms_] = jarr_forms;
        // templates — всегда пишем
        nlohmann::json jarr_templates = nlohmann::json::array();
        for (const auto& tmpl : m_start_load_templates_)
            jarr_templates.emplace_back(tmpl);
        j_start[pch_json_start_templates_] = jarr_templates;
        // load — только если пользователь что-то загружал
        if (!m_start_load_file_templates_.empty()) {
            nlohmann::json jarr_load = nlohmann::json::array();
            for (const auto& [file, tmpl] : m_start_load_file_templates_) {
                nlohmann::json j_entry;
                j_entry[pch_json_start_load_file_]     = file;
                j_entry[pch_json_start_load_template_] = tmpl;
                jarr_load.emplace_back(j_entry);
            }
            j_start[pch_json_start_load_] = jarr_load;
        }

        nlohmann::json j_file;
        j_file[pch_json_start_] = j_start;

        spdlog::info("write JSON file: [{}]", path.toStdString());

        QFile file(path);
        // Создаём директорию если не существует
        QFileInfo(file).dir().mkpath(".");

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            spdlog::critical("Can't open file for write: [{}]", path.toStdString());
            return false;
        }

        const std::string dump = j_file.dump(1, ' ');
        file.write(dump.data(), static_cast<qint64>(dump.size()));

    } catch (const std::exception& ex) {
        exclog(ex);
        return false;
    } catch (...) {
        exclog();
        return false;
    }
    return true;
}

bool RastrParameters::templ_sort_func(const std::pair<std::string,std::string>& p1,
                             const std::pair<std::string,std::string>& p2)
{
    return (p1.first.length() < p2.first.length());
}

bool RastrParameters::readTemplates(){
    try {
        m_template_exts_.clear();

        // QDirIterator корректно обходит директории с кириллицей
        // на всех платформах
        QDirIterator it(dir_SHABLON_.absolutePath(),
                        QDir::Files | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            const QFileInfo fi = it.fileInfo();
            m_template_exts_.emplace_back(
                fi.baseName().toStdString(),
                ("." + fi.suffix()).toStdString());
        }
        std::sort(m_template_exts_.begin(), m_template_exts_.end(),
                  templ_sort_func);

    } catch (const std::exception& ex) {
        exclog(ex);
        return false;
    } catch (...) {
        exclog();
        return false;
    }
    return true;
}

bool RastrParameters::readFormsExists(){
    try {
        m_forms_exists_.clear();

        QDirIterator it(dir_forms_.absolutePath(),
                        {"*.fm"},          // фильтр по расширению — без ручной проверки
                        QDir::Files | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            m_forms_exists_.emplace_back(
                it.fileInfo().fileName().toStdString());
        }

    } catch (const std::exception& ex) {
        exclog(ex);
        return false;
    } catch (...) {
        exclog();
        return false;
    }
    return true;
}