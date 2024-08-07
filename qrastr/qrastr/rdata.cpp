#include "rdata.h"

void RData::Initialize(CUIForm _form)
{
    //Мета информация о таблице (по сути шаблон)
    std::string str_json;
    str_json.resize(CRastrHlp::SIZE_STR_BUF_);
    int nRes = GetMeta( id_rastr_, t_name_.c_str(), "", const_cast<char*>(str_json.c_str()), static_cast<long>(str_json.size()) );
    if(nRes<0){
        qDebug() << "GetMeta(...)  return error" << nRes;
    }
    //qDebug() << "Meta   : " << str_json.c_str();
    str_json.resize(std::strlen(str_json.c_str())+1);
    j_metas_ = nlohmann::json::parse(str_json);
    reserve(_form.Fields().size()+5);               // Без reserve RCol данные обнуляются видимио при reallocation  If a reallocation happens, all contained elements are modified.

    // В RData создаем RCol по образу формы
    for (CUIFormField &f : _form.Fields()){
      for(const nlohmann::json& j_meta : j_metas_ ){
        const std::string str_Name = j_meta["Name"];
        if(f.Name() == str_Name){ // for make same order like in a form
            str_cols_.append(f.Name());
            str_cols_.append(",");

            RCol rc;
            rc.str_name_ = f.Name();
            rc.setMeta( j_meta );

            int nRes = AddCol(rc); Q_ASSERT(nRes>=0);
            break;
        }
      }
    }

    if(str_cols_.length()>0)
        str_cols_.pop_back();
    qDebug() << "Open Table : " << t_name_.c_str();
    qDebug() << "Fields : " << str_cols_.c_str();
}

void RData::Initialize(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form)
{
    std::string str_tmp;
    std::string str_json;

    for(const nlohmann::json& j_field : _j_Fields){
        for(const nlohmann::json& j_meta : _j_metas ){
            const std::string str_Name = j_meta["Name"];
            if(j_field == str_Name){ // for make same order like in a form
                RCol rc;
                rc.str_name_ = str_Name;
                rc.setMeta( j_meta );

                int nRes = AddCol(rc); Q_ASSERT(nRes>=0);
                break;
            }
        }
    }
}

void RData::populate()
{
    std::string str_json;
    str_json.resize(CRastrHlp::SIZE_STR_BUF_);
    GetJSON(id_rastr_,t_name_.c_str(), str_cols_.c_str(),"","",const_cast<char*>(str_json.c_str()), static_cast<long>(str_json.length()));

    str_json.resize(std::strlen(str_json.c_str())+1);
    //qDebug() << "Data: " << str_json.c_str();
    //size_t nLength = str_json.size();
    nlohmann::json j_data_arr = nlohmann::json::parse(str_json);
    size_t sz_num_rows = j_data_arr.size();
    for( RCol& rcol : *this ){
        rcol.resize(sz_num_rows);
    }

    int n_row = 0;
    int i = 0;


    for(nlohmann::json j_data_row : j_data_arr) {
        i = 1;
        for(RCol& col: *this){
            //qDebug() << "D: " << j_data_row[i].dump().c_str();
            RCol::iterator iter_col = col.begin() + n_row;
            //qDebug() << "dump: " << j_data_row[i].dump().c_str();
            switch(col.en_data_){
            case RCol::_en_data::DATA_INT:
                (*iter_col).emplace<int>(  j_data_row[i] );
                //qDebug() << "int: " << std::get<int>(*iter_col);
                break;
            case RCol::_en_data::DATA_DBL:
                (*iter_col).emplace<double>( j_data_row[i] );
                //qDebug() << "dbl: " << std::get<double>(*iter_col);
                break;
            case RCol::_en_data::DATA_STR:
                (*iter_col).emplace<std::string>(j_data_row[i]);
                //qDebug() << "str: " << std::get<std::string>(*iter_col).c_str();
                break;
            default:
                Q_ASSERT(!"unknown type");
                break;
            }//switch
            i++;
        }//for(col)
        n_row++;
    }//for(j_data_arr)
}

void RData::populate(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form)
{
    // clear_data();
    std::string str_tmp;
    std::string str_json;
    int nRes = 0;

    _vstr vstr_fields_meta;
    str_tmp.clear();
    for(const nlohmann::json& j_meta : _j_metas ){
        std::string str_Name       = j_meta["Name"];
        vstr_fields_meta.emplace_back(str_Name);
        str_tmp += str_Name;
        str_tmp += " # ";
    }

    qDebug() << "FieldsBd:  " << str_tmp.c_str();
    _vstr vstr_fields_distilled;
    str_tmp.clear();
    for(std::string& str_field_form : _vstr_fields_form){
        _vstr::const_iterator iter_vstr_fields_meta =
            std::find(vstr_fields_meta.begin(), vstr_fields_meta.end(), str_field_form );
        if(iter_vstr_fields_meta != vstr_fields_meta.end()){
            vstr_fields_distilled.emplace_back(str_field_form);
            str_tmp += str_field_form;
            str_tmp += ",";
        }
    }


    int i = 0;
    std::string str_tmp2 = getCommaSeparatedFieldNames();
    qDebug() << "Fields:  " << str_tmp.c_str();
    qDebug() << "Fields2: " << str_tmp2.c_str();
    if(str_tmp.length()>0){
        str_tmp.erase(str_tmp.length()-1);
        str_json.resize(SIZE_STR_BUF);
        //nRes = GetJSON( id_rastr_, str_TableName.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
        nRes = GetJSON( id_rastr_, t_name_.c_str(), str_tmp.c_str(), "","", const_cast<char*>(str_json.c_str()), str_json.length() );
        str_json.resize(std::strlen(str_json.c_str())+1);
        //qDebug() << "Data: " << str_json.c_str();
        size_t nLength = str_json.size();
        //nlohmann::json j_data_arr = nlohmann::json::parse(str_json, nullptr,false);
        nlohmann::json j_data_arr = nlohmann::json::parse(str_json);
        size_t sz_num_cols = this->size();
        size_t sz_num_rows = j_data_arr.size();
        for( RCol& rcol : *this ){
            rcol.clear();
            rcol.resize(sz_num_rows);
        }
        int n_row = 0;



        for(nlohmann::json j_data_row : j_data_arr) {
            i = 1;
            for(RCol& col: *this){
                //qDebug() << "D: " << j_data_row[i].dump().c_str();
                RCol::iterator iter_col = col.begin() + n_row;
                qDebug() << "dump: " << j_data_row[i].dump().c_str();
                switch(col.en_data_){
                case RCol::_en_data::DATA_INT:
                    (*iter_col).emplace<int>(  j_data_row[i] );
                    //qDebug() << "int: " << std::get<int>(*iter_col);
                    break;
                case RCol::_en_data::DATA_DBL:
                    (*iter_col).emplace<double>( j_data_row[i] );
                    //qDebug() << "dbl: " << std::get<double>(*iter_col);
                    break;
                case RCol::_en_data::DATA_STR:
                    (*iter_col).emplace<std::string>(j_data_row[i]);
                    //qDebug() << "str: " << std::get<std::string>(*iter_col).c_str();
                    break;
                default:
                    Q_ASSERT(!"unknown type");
                    break;
                }//switch
                i++;
            }//for(col)
            n_row++;
        }//for(j_data_arr)
    }
}

void RData::clear_data()
{
    for(RCol& col: *this){
        col.clear();
    }
}

int RData::AddRow(int index )
{
    //index default = -1
    // TO DO: make same action on astra (server)
    TableInsRow(id_rastr_,t_name_.c_str(),index);

   /* _vt val;
    if ( (index < 0) || (index > (*this)[0].size() )) // add at end
    {
        for( RCol& col : *this )
        {
            col.push_back(val);
        }
    }
    else
    {
        for( RCol& col : *this )
        {
            col.insert(col.begin()+index,val);
        }
    }*/
    return 1;
}

int RData::RemoveRDMRow(int index )
{
    // TO DO: make same action on astra (server)
    TableDelRow(id_rastr_,t_name_.c_str(),index);

   /* for( RCol& col : *this )
    {
        col.erase(col.begin()+index);
    }
    */
    return 1;
}
