#include "rastrhlp.h"
#include "common.h"
#include "utils.h"
#include "License2/json.hpp"

CRastrHlp::CRastrHlp(){

}

bool CRastrHlp::IsIdValid( _idRastr id ){
    if(id<0)
        return false;
    return true;
};

int CRastrHlp::CreateRastr(){
    id_rastr_ = RastrCreate();
    if(!IsIdValid(id_rastr_))
        throw CException("invalid rastr id: {}",id_rastr_);
    return id_rastr_;
}

CRastrHlp::~CRastrHlp(){
    if(IsIdValid(id_rastr_))
        ::RastrExterminate(id_rastr_);
}

int CRastrHlp::Load(std::string str_path_to_file){
    try{
        int nRes = 0;
        std::filesystem::path path_file_load;
        if( !std::filesystem::exists(str_path_to_file.c_str())) {
            spdlog::error( "File rastr  not exist : [{}]!", str_path_to_file );
            return -1;
        }
        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        path_file_load = stringutils::utf8_decode(str_path_to_file);
        nRes = ::Load(id_rastr_, path_file_load.c_str(), L"");
        if(nRes<0){
            throw CException("Can't read Rastr file: {}", str_path_to_file);
        }else{
            qDebug() << "File : [" <<str_path_to_file.c_str() << "] loaded.";
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
int CRastrHlp::Save(std::string str_path_to_file){
    try{
        int nRes = 0;
        std::filesystem::path path_file_save;
        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        path_file_save = stringutils::utf8_decode(str_path_to_file);
        nRes = ::Save(id_rastr_, str_path_to_file.c_str(), "");
        if(nRes<0){
            throw CException("can't save Rastr file: {}", str_path_to_file);
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

// form files are deployed in form catalog near qrastr.exe
int CRastrHlp::ReadForms(std::string str_path_forms){
    try{
        std::vector<std::string> forms = split(str_path_forms, ',');

        std::filesystem::path path_forms ("form");
        //std::filesystem::path path_forms_load;
        std::filesystem::path path_form_load;
#if(defined(_MSC_VER))
        //str_path_forms_load = R"(C:\Users\ustas\Documents\RastrWin3\form\poisk.fm)";
        //str_path_forms_load = R"(C:\Users\ustas\Documents\RastrWin3\form\Общие.fm)";
        //str_path_forms_load = R"(C:\projects\astra\Общие.fm)";

        //on Windows, you MUST use 8bit ANSI (and it must match the user's locale) or UTF-16 !! Unicode!
        //!!! https://stackoverflow.com/questions/30829364/open-utf8-encoded-filename-in-c-windows  !!!
        // path_forms_load = stringutils::utf8_decode(str_path_to_file_forms);
        //upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>(CUIFormCollectionSerializerBinary(path_form_load).Deserialize());

        upCUIFormsCollection_ = std::make_unique<CUIFormsCollection>();
        for (std::string &form : forms)
        {
            std::filesystem::path path_file_form = stringutils::utf8_decode(form);
            path_form_load =  path_forms / path_file_form;

            qDebug() << "read form from file : " << path_form_load.wstring();
    #else
            path_forms_load = str_path_to_file_forms;
            qDebug() << "read form from file : " << path_forms_load.c_str();
    #endif
            CUIFormsCollection CUIFormsCollection_ = CUIFormCollectionSerializerBinary(path_form_load).Deserialize();
            for(const  CUIForm& uiform : CUIFormsCollection_.Forms()){

                //qDebug() << "form : " << uiform.TableName().c_str();
                upCUIFormsCollection_->Forms().emplace_back(uiform);

            }
          //  vupCUIFormsCollection_.push_back(upCUIFormsCollection_);

            //std::string str_json;
            //str_json.resize(SIZE_STR_BUF_);
           // int  nRes = ::GetForms( reinterpret_cast<const char*>(str_path_to_file_forms.c_str()), "", const_cast<char*>(str_json.c_str()), str_json.size() );
           // int  nRes = ::GetForms( reinterpret_cast<const wchar_t*>(str_path_to_file_forms.c_str()), reinterpret_cast<const wchar_t*>(""), const_cast<char*>(str_json.c_str()), str_json.size() );
            //str_jforms_ = str_json;
            //jforms_ = nlohmann::json::parse(str_json);
        }
        qDebug() << "Thats all forms.\n" ;

/*
        nlohmann::json j_forms;
        //CUIFormsCollection uifc{CUIFormCollectionSerializerBinary(path_forms_load).Deserialize()};
        for(const  CUIForm& uiform : uifc.Forms() ){
            nlohmann::json j_form;
            j_form["TableName"]  = uiform.TableName();
            j_form["Name"]       = stringutils::cp1251ToUtf8( uiform.Name() );// acp_decode
            j_form["Collection"] = stringutils::cp1251ToUtf8( uiform.Collection() );
            j_form["MenuPath"]   = stringutils::cp1251ToUtf8( uiform.MenuPath() );
            j_form["Query"]      = uiform.Query();
            nlohmann::json j_fields;
            for( const CUIFormField &iter_l_uiform_fields : uiform.Fields() ){
                j_fields.emplace_back( iter_l_uiform_fields.Name() );
            }
            j_form["Fields"] = j_fields;
            j_forms.emplace_back(j_form);
        }
        std::string str_data = j_forms.dump( 1,' ', true, nlohmann::json::error_handler_t::ignore ); // nlohmann::json::error_handler_t::ignore
*/
/*

        int nRes = 0;
        std::string str_json;
        str_json.resize(SIZE_STR_BUF_);
        nRes = ::GetForms( str_path_to_file_forms.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.size() );
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
            nRes = ::GetMeta( id_rastr_, str_TableName.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
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
                nRes = ::GetJSON( id_rastr_, str_TableName.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
                qDebug() << "Data: " << str_json.c_str();
            }
        }
*/
    }catch(const std::exception& ex){
        exclog(ex);
        return -1;
    }catch(...){
        exclog();
        return -2;
    }
    return 1;
}

std::list<CUIForm> CRastrHlp::GetForms() const {
    return upCUIFormsCollection_->Forms();
};

CUIForm CRastrHlp::GetUIForm(size_t n_form_indx){
    auto it = upCUIFormsCollection_->Forms().begin();
    std::advance(it,n_form_indx);
    auto form  =*it;

    return form;
}

int CRastrHlp::GetFormData(int n_form_indx){
    if(n_form_indx<0)
        return -1;
    auto it = upCUIFormsCollection_->Forms().begin();
    std::advance(it,n_form_indx);
    auto form  =*it;

    return 1;
/*
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

*/
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
