#include "rdata.h"
#include "iostream"

template<typename T>
class DataBlock : public IRastrDataBlock<T>
{
protected:
    T* Block_ = nullptr;
    long Rows_ = 0;
    long Columns_ = 0;
    long* pIndiciesChanged_ = nullptr;
    std::list<std::string> ColumnTitles_;
    std::vector<long> vChangedIndices_;

public:
    IPlainRastrRetCode SetBlockSize(long Rows, long Columns) noexcept override
    {
        if (Block_ != nullptr)
            delete[] Block_;

        ColumnTitles_.clear();

        Rows_ = Columns_ = 0;
        Block_ = new T[Rows * Columns];
        if (Block_ != nullptr)
        {
            Rows_ = Rows;
            Columns_ = Columns;
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }
    ~DataBlock()
    {
        delete[] Block_;
    }

    // отдаем SparseDataBlock в сервер

    // Говорим сколько вообще значений в DataBlock
    long ValuesAvailable() const noexcept override
    {
        return Rows_ * Columns_;
    }

    // Возвращаем значение в диапазоне [0; ValuesAvailable)
    // в структуре (строка, столбец, const* значение, код возврата)
    IRastrDataBlock<T>::ValueReturn Value(long Index) const noexcept override
    {
        // здесь демо возврата просто прямоугольной таблицы
        // если есть строки / столбцы / инфа о дефолтах внутри датаблока - отдаем из списка
        // по коду возврата можно закенселить, если хочется.
        //return { Index / Columns(), Index % Columns(), Block_ + Index, IPlainRastrRetCode::Ok };
        // если вернуть из Value NotImplemented - будет предпринята попытка
        // получить dense блок - см. ниже
        return IRastrDataBlock<T>::Value(Index);
    }

    // Ну а если блок dense - то отдаем

    const T* Data() const override { return Block_; }
    T* Data() override { return Block_; }
    long Rows() const override { return Rows_; }
    long Columns() const override { return Columns_; }
    const long* ChangedIndices() const override { return vChangedIndices_.data(); }
    const size_t ChangedIndicesCount() const override { return vChangedIndices_.size(); }


    // и забираем просто из прямоугольной таблицы


    IPlainRastrRetCode Emplace(long Row, long Column, const T& Value) noexcept override
    {
        if (Row >= 0 && Row < Rows_ && Column >= 0 && Column < Columns_)
        {
            Block_[Row * Columns_ + Column] = Value;
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }
    IPlainRastrRetCode EmplaceSaveIndChange(long Row, long Column, const T& Value) noexcept override
    {
        if (Emplace(Row, Column, Value) != IPlainRastrRetCode::Failed)
        {
            vChangedIndices_.push_back(Row * Columns_ + Column);
            return IPlainRastrRetCode::Ok;
        }
        else
            return IPlainRastrRetCode::Failed;
    }

    IPlainRastrRetCode MapColumn(const std::string_view Name, long DataBlockIndex) const noexcept override
    {
        const_cast<DataBlock*>(this)->ColumnTitles_.emplace_back(Name);
        return IPlainRastrRetCode::Ok;
    }

    struct ToString {
        std::string operator()(std::monostate) { return { "def" }; }
        std::string operator()(const long& value) { return std::to_string(value); }
        std::string operator()(const uint64_t& value) { return std::to_string(value); }
        std::string operator()(const double& value) { return std::to_string(value); }
        std::string operator()(const bool& value) { return value ? "1" : "0"; }
        std::string operator()(const std::string& value) { return value; }
    };
    struct ToVal {
        std::string operator()(std::monostate) { return { "def" }; }
        long operator()(const long& value) { return value; }
        std::string operator()(const uint64_t& value) { return std::to_string(value); }
        std::string operator()(const double& value) { return std::to_string(value); }
        std::string operator()(const bool& value) { return value ? "1" : "0"; }
        std::string operator()(const std::string& value) { return value; }
    };

    void Dump()
    {
        for (const auto& ColumnTitle : ColumnTitles_)
            std::cout << ColumnTitle << ";";
        std::cout << std::endl;
        for (long row = 0; row < Rows(); row++)
        {
            for (long column = 0; column < Columns(); column++)
                std::cout << StringValue(Data()[row * Columns() + column]) << ";";
            std::cout << std::endl;
        }
    }
    void QDump()
    {
        std::string str_cols = "";
        for (const auto& ColumnTitle : ColumnTitles_)
        {
            str_cols.append(ColumnTitle);
            str_cols.append(";");
        }
        qDebug() << str_cols;
        for (long row = 0; row < Rows(); row++)
        {
            std::string str_out = "";
            for (long column = 0; column < Columns(); column++)
            {
                str_out.append(StringValue(Data()[row * Columns() + column]));
                str_out.append(";");
            }
            qDebug() << str_out << ";";
        }
    }

protected:
    std::string StringValue(const double& Value)
    {
        return std::to_string(Value);
    }

    std::string StringValue(const FieldVariantData& Value)
    {
        return std::visit(ToString(), Value);
    }
};
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

void RData::populate_qastra(QAstra* _pqastra)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(t_name_) };
    IRastrColumnsPtr nodecolumns{ table->Columns() };
    DataBlock<FieldVariantData> nparray;
    IRastrResultVerify(table->DenseDataBlock(str_cols_, nparray, Options));

    //nparray.QDump();

    for (long column = 0; column < nparray.Columns(); column++)
    {
        RCol& rcol = (*this)[column];
        rcol.resize(nparray.Rows());
        //RCol::iterator iter_col = this->begin()+column;
        for (long row = 0; row < nparray.Rows(); row++)
        {
            RCol::iterator iter_col = rcol.begin() + row;
            switch(rcol.en_data_){
            case RCol::_en_data::DATA_BOOL:
                (*iter_col).emplace<bool>(std::get<bool>(nparray.Data()[row * nparray.Columns() + column]));
                break;
            case RCol::_en_data::DATA_INT:
                (*iter_col).emplace<long>(std::get<long>(nparray.Data()[row * nparray.Columns() + column]));
                break;
            case RCol::_en_data::DATA_DBL:
               // (*iter_col).emplace<double>( j_data_row[i] );
                (*iter_col).emplace<double>(std::get<double>(nparray.Data()[row * nparray.Columns() + column]));
                //qDebug() << "dbl: " << std::get<double>(*iter_col);
                break;
            case RCol::_en_data::DATA_STR:
                (*iter_col).emplace<std::string>(std::get<std::string>(nparray.Data()[row * nparray.Columns() + column]));
               // (*iter_col).emplace<std::string>(j_data_row[i]);
                //qDebug() << "str: " << std::get<std::string>(*iter_col).c_str();
                break;
            default:
                Q_ASSERT(!"unknown type");
                break;
            }//switch
        }
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
