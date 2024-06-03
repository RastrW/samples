#include "rastrdatamodel.h"

#if(!defined(QICSGRID_NO))

void RData::Initialize(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form)
{
    std::string str_tmp;
    std::string str_json;
    int nRes = 0;

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
        nRes = GetJSON( id_rastr_, t_name_.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
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

    _vt val;
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
    }
    return 1;
}

int RData::RemoveRDMRow(int index )
{
    // TO DO: make same action on astra (server)

    for( RCol& col : *this )
    {
        col.erase(col.begin()+index);
    }
    return 1;
}
#endif// #if(!defined(QICSGRID_NO))

