#include "rdata.h"
#include "iostream"


void RData::Initialize(CUIForm _form, QAstra* _pqastra)
{
    qDebug() << "Start RData::Initialize t_name : " << t_name_.c_str();

    IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(t_name_) };
    IRastrColumnsPtr columns{ table->Columns() };
    //std::string table_Name = IRastrPayload(IRastrVariantPtr(table->Name()));

    // В RData создаем RCol по образу формы
    /*for (CUIFormField &f : _form.Fields()){
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
    }*/

    IRastrPayload ColumnsCount{ columns->Count() };
    qDebug() << "ColumnsCount : " << ColumnsCount.Value()+5;
    reserve( ColumnsCount.Value()+5);                // Без reserve RCol данные обнуляются видимио при reallocation  If a reallocation happens, all contained elements are modified.
    qDebug() << "reserve : " << ColumnsCount.Value()+5 <<" ok";

    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Type = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Type))->String()).Value();
        std::string col_Name = IRastrPayload(col->Name()).Value();
        //qDebug() << "index : " << index << " col_Type " << col_Type << " col_Name " << col_Name;

        str_cols_.append(col_Name);
        str_cols_.append(",");

        RCol rc;
        rc.str_name_ = col_Name;
        rc.table_name_ = t_name_;
        rc.index = index;
        rc.setMeta(_pqastra);
        rc.hidden = true;

        int nRes = AddCol(rc);
        Q_ASSERT(nRes>=0);
        if (mCols_.find(col_Name) == mCols_.end())
            mCols_.insert(std::pair(col_Name,index));
    }

    for (CUIFormField &f : _form.Fields())
    {
        for  (RCol &rc : *this)
        {
            if (rc.str_name_ == f.Name())
                rc.hidden = false;
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
                (*iter_col).emplace<long>(  j_data_row[i] );
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

//obsolete
/*void RData::populate(nlohmann::json _j_Fields , nlohmann::json _j_metas,_vstr _vstr_fields_form)
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
        nRes = GetJSON( id_rastr_, t_name_.c_str(), str_tmp.c_str(), "","", const_cast<char*>(str_json.c_str()), static_cast<long>(str_json.length()) );
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
                    (*iter_col).emplace<long>(  j_data_row[i] );
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
*/

void RData::populate_qastra(QAstra* _pqastra, RTablesDataManager* _pRTDM )
{
    /*
     * Идея иметь 1 хранилище данных таблицы для всех клиентов
     * Теперь попробуем обратиться к менеджеру данных таблиц
     * если такая таблица уже есть берем указатель на нее, если нет
     * тогда создаем в менеджере и отдаем указатель
    */
    pnparray_ = _pRTDM->Get(t_name_,str_cols_);

   /* FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);
    IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(t_name_) };

    //DataBlock<FieldVariantData> nparray;

    IRastrResultVerify(table->DataBlock(str_cols_, nparray_, Options));*/

    //nparray_.QDump(20);

    //IRastrResultVerify(table->SparseDataBlock(str_cols_, nparray,Options));   // падает эл 0;0 - monostate , а кастится к bool
    //IRastrResultVerify(table->DataSet(str_cols_, nparray_, Options));

    //const long* pIndicesChanged =  nparray_.ChangedIndices();            // массив записанных индексов (0...ValuesAvailable)
   // const size_t IndicesChangedCount = nparray_.ChangedIndicesCount();	// размер массива измененных индексов
    //nparray.QDump();

    // Test populate with Sparse DataBlock
    /*
    //Создаем колонки инициализированные дефолтными значениями
    for (long column = 0; column < nparray.Columns(); column++)
    {
        RCol& rcol = (*this)[column];
        rcol.index = column;

        IRastrColumnPtr col_ptr{ nodecolumns->Item(rcol.name()) };
        std::string prop_nameref = IRastrPayload(IRastrVariantPtr(col_ptr->Property(FieldProperties::NameRef))->String()).Value();
        rcol.nameref = prop_nameref;
        rcol.resize(nparray.Rows());
    }
    //заполняем значениям
    for (long column = 0; column < nparray.Columns(); column++)
    {
        RCol& rcol = (*this)[column];
        for (long ich_indx = 0; ich_indx < IndicesChangedCount; ich_indx++)
        {
            long ch_indx = pIndicesChanged[ich_indx];
            long row = ch_indx / nparray.Columns();
            RCol::iterator iter_col = rcol.begin() + row;
            switch(rcol.en_data_){
            case RCol::_en_data::DATA_BOOL:
                (*iter_col).emplace<bool>(std::get<bool>(nparray.Data()[ch_indx]));
                break;
            case RCol::_en_data::DATA_INT:
                (*iter_col).emplace<long>(std::get<long>(nparray.Data()[ch_indx]));
                break;
            case RCol::_en_data::DATA_DBL:
                (*iter_col).emplace<double>(std::get<double>(nparray.Data()[ch_indx]));
                break;
            case RCol::_en_data::DATA_STR:
                (*iter_col).emplace<std::string>(std::get<std::string>(nparray.Data()[ch_indx]));
                break;
            default:
                Q_ASSERT(!"unknown type");
                break;
            }//switch
        }
    }
    */

    //Populate with Dense Data Block
    /*
    for (long column = 0; column < nparray_.Columns(); column++)
    {
        RCol& rcol = (*this)[column];
        rcol.index = column;


        rcol.resize(nparray_.Rows());

        for (long row = 0; row < nparray_.Rows(); row++)
        {
            RCol::iterator iter_col = rcol.begin() + row;
            switch(rcol.en_data_){
            case RCol::_en_data::DATA_BOOL:
                (*iter_col).emplace<bool>(std::get<bool>(nparray_.Data()[row * nparray_.Columns() + column]));
                break;
            case RCol::_en_data::DATA_INT:
                (*iter_col).emplace<long>(std::get<long>(nparray_.Data()[row * nparray_.Columns() + column]));
                break;
            case RCol::_en_data::DATA_DBL:
                (*iter_col).emplace<double>(std::get<double>(nparray_.Data()[row * nparray_.Columns() + column]));
                break;
            case RCol::_en_data::DATA_STR:
                (*iter_col).emplace<std::string>(std::get<std::string>(nparray_.Data()[row * nparray_.Columns() + column]));
                break;
            default:
                Q_ASSERT(!"unknown type");
                break;
            }//switch
        }
    }
*/
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
   // TableInsRow(id_rastr_,t_name_.c_str(),index);

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
   // TableDelRow(id_rastr_,t_name_.c_str(),index);

   /* for( RCol& col : *this )
    {
        col.erase(col.begin()+index);
    }
    */
    return 1;
}
