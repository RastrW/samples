#include "rastrhlp.h"
#include "common.h"
#include "License2/json.hpp"

CRastrHlp::CRastrHlp(){

}

CRastrHlp::~CRastrHlp(){
    RastrExterminate(id_rastr_);

}

bool CRastrHlp::IsIdValid( _idRastr id ){
    if(id<0)
        return false;
    return true;
};

int CRastrHlp::CreateRastr(){
    id_rastr_ = RastrCreate();
    if(!IsIdValid(id_rastr_))
        throw std::exception(fmt::format("invalid rastr id: {}",id_rastr_).c_str());
    return id_rastr_;
}

int CRastrHlp::ReadForms(std::string str_path_to_file_forms){
    try{
        int nRes = 0;
        std::string str_json;
        str_json.resize(SIZE_STR_BUF_);
        nRes = GetForms( str_path_to_file_forms.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.size() );
        if(nRes<0){
            throw std::exception(fmt::format("can't read file: {}", str_path_to_file_forms).c_str());
        }
        nlohmann::json j_forms = nlohmann::json::parse(str_json);
        //sqDebug() << str_json.c_str();
        typedef std::vector<std::string> _vstr;
        for(const nlohmann::json& j_form : j_forms ){
            std::string str_TableName  = j_form["TableName"];
            std::string str_Name       = j_form["Name"];
            std::string str_Collection = j_form["Collection"];
            std::string str_MenuPath   = j_form["MenuPath"];
            std::string str_Query      = j_form["Query"];
            nlohmann::json j_Fields    = j_form["Fields"];
            _vstr vstr_fields_form;
            std::string str_tmp;
            for(const nlohmann::json& j_field : j_Fields){
                vstr_fields_form.emplace_back(j_field);
                str_tmp += j_field;
                str_tmp += " # ";
            }
            qDebug() << "Table  : " << str_TableName.c_str() << "|" << QString::fromUtf8( str_Name.c_str()) << "|" <<  str_Collection.c_str() << "|" << str_MenuPath.c_str() << "|" << str_Query.c_str();
            qDebug() << "Fields : " << str_tmp.c_str();
            nRes = GetMeta( id_rastr_, str_TableName.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
            if(nRes<0){
                qDebug() << "GetMeta(...)  return error" << nRes;
                continue;
            }
            //qDebug() << "Meta   : " << str_json.c_str();
            nlohmann::json j_metas = nlohmann::json::parse(str_json);
            _vstr vstr_fields_meta;
            str_tmp.clear();
            for(const nlohmann::json& j_meta : j_metas ){
                std::string str_Name       = j_meta["Name"];
                vstr_fields_meta.emplace_back(str_Name);
                str_tmp += str_Name;
                str_tmp += " # ";
            }
            qDebug() << "FieldsBd: " << str_tmp.c_str();
            _vstr vstr_fields_distilled;
            str_tmp.clear();
            for(std::string& str_field_form : vstr_fields_form){
                _vstr::const_iterator iter_vstr_fields_meta =
                    std::find(vstr_fields_meta.begin(), vstr_fields_meta.end(), str_field_form );
                if(iter_vstr_fields_meta != vstr_fields_meta.end()){
                    vstr_fields_distilled.emplace_back(str_field_form);
                    str_tmp += str_field_form;
                    str_tmp += ",";
                }
            }
            if(str_tmp.length()>0){
                str_tmp.erase(str_tmp.length()-1);
                nRes = GetJSON( id_rastr_, str_TableName.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
                qDebug() << "Data: " << str_json.c_str();
            }
        }
    }catch(const std::exception& ex){
        exclog(ex);
        return -1;
    }catch(...){
        exclog();
        return -2;
    }
    return 1;
}


/*

    const int SIZE_STR_BUF = 100'000;
    std::string str_json;
    str_json.resize(SIZE_STR_BUF);
    //nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/poisk.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/Общие.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    if(nRes<0){
        QMessageBox mb;
        mb.setText("forms not loaded!");
        mb.exec();
        return 13;
    }
    nlohmann::json j_forms = nlohmann::json::parse(str_json);
    //sqDebug() << str_json.c_str();
    typedef std::vector<std::string> _vstr;
    for(const nlohmann::json& j_form : j_forms ){

        std::string str_TableName  = j_form["TableName"];
        std::string str_Name       = j_form["Name"];
        std::string str_Collection = j_form["Collection"];
        std::string str_MenuPath   = j_form["MenuPath"];
        std::string str_Query      = j_form["Query"];

        nlohmann::json j_Fields = j_form["Fields"];
        _vstr vstr_fields_form;
        std::string str_tmp;
        for(const nlohmann::json& j_field : j_Fields){
            vstr_fields_form.emplace_back(j_field);
            str_tmp += j_field;
            str_tmp += " # ";
        }
        qDebug() << "Table  : " << str_TableName.c_str() << "|" << QString::fromUtf8( str_Name.c_str()) << "|" <<  str_Collection.c_str() << "|" << str_MenuPath.c_str() << "|" << str_Query.c_str();
        qDebug() << "Fields : " << str_tmp.c_str();
        nRes = GetMeta( id_rastr, str_TableName.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
        if(nRes<0){
             qDebug() << "GetMeta(...)  return error" << nRes;
             continue;
        }
        //qDebug() << "Meta   : " << str_json.c_str();
        nlohmann::json j_metas = nlohmann::json::parse(str_json);
        _vstr vstr_fields_meta;
        str_tmp.clear();
        for(const nlohmann::json& j_meta : j_metas ){
            std::string str_Name       = j_meta["Name"];
            vstr_fields_meta.emplace_back(str_Name);
            str_tmp += str_Name;
            str_tmp += " # ";
        }
        qDebug() << "FieldsBd: " << str_tmp.c_str();
        _vstr vstr_fields_distilled;
        str_tmp.clear();
        for(std::string& str_field_form : vstr_fields_form){
            _vstr::const_iterator iter_vstr_fields_meta =
                std::find(vstr_fields_meta.begin(), vstr_fields_meta.end(), str_field_form );
            if(iter_vstr_fields_meta != vstr_fields_meta.end()){
                vstr_fields_distilled.emplace_back(str_field_form);
                str_tmp += str_field_form;
                str_tmp += ",";
            }
        }
        if(str_tmp.length()>0){
            str_tmp.erase(str_tmp.length()-1);
            nRes = GetJSON( id_rastr, str_TableName.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
            qDebug() << "Data: " << str_json.c_str();
        }


    }


*/