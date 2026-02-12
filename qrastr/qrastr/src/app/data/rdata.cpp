#include "rdata.h"

void RData::Initialize(CUIForm _form, QAstra* _pqastra)
{
    t_title_ = _form.Name().c_str();

    IRastrTablesPtr tablesx{ _pqastra->getRastr()->Tables() };
    IRastrPayload res{ tablesx->FindIndex(t_name_) };
    int t_ind = res.Value();
    if (t_ind < 0)
        return;

    IRastrTablePtr table{ tablesx->Item(t_name_) };
    IRastrColumnsPtr columns{ table->Columns() };

    IRastrPayload ColumnsCount{ columns->Count() };
    long n_reserve = ColumnsCount.Value()+5;
    qDebug() << "ColumnsCount : " << ColumnsCount.Value();
    reserve( n_reserve);                // Без reserve RCol данные обнуляются видимио при reallocation  If a reallocation happens, all contained elements are modified.
    qDebug() << "reserve : " << n_reserve <<" ok";

    str_cols_ = "";
    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Type = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Type))->String()).Value();
        std::string col_Name = IRastrPayload(col->Name()).Value();
        std::string col_Title = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Title))->String()).Value();

        vCols_.push_back(col_Name);
        str_cols_.append(col_Name);
        str_cols_.append(",");

        RCol rc;
        rc.str_name_ = col_Name;
        rc.table_name_ = t_name_;
        rc.title_ = col_Title;
        rc.index = index;
        rc.setMeta(_pqastra);
        rc.hidden = true;

        int nRes = AddCol(rc);
        Q_ASSERT(nRes>=0);
        if (mCols_.find(col_Name) == mCols_.end())
            mCols_.insert(std::pair(col_Name,index));
    }

    //Скрыть колонки не входящие в форму
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
    throw  std::runtime_error("called absolete function");
}

void RData::populate_qastra(QAstra* _pqastra, RTablesDataManager* _pRTDM )
{
    /*
     * Идея иметь 1 хранилище данных таблицы для всех клиентов
     * Теперь попробуем обратиться к менеджеру данных таблиц
     * если такая таблица уже есть берем указатель на нее, если нет
     * тогда создаем в менеджере и отдаем указатель
    */
    pnparray_ = _pRTDM->get(t_name_,str_cols_);
}

std::string RData::get_cols(bool visible)
{
    std::string ret_cols="";
    if (visible)
    {
        for  (RCol &rc : *this)
            if (!rc.hidden)
                ret_cols += rc.str_name_ + ",";
    }
    else
    {
        for  (RCol &rc : *this)
            ret_cols += rc.str_name_ + ",";
    }

    if (!ret_cols.empty())
        ret_cols.pop_back();

    return ret_cols;
}

void RData::clear_data(){
    for(RCol& col: *this){
        col.clear();
    }
}

int RData::AddRow(int index ){
    return 1;
}

int RData::RemoveRDMRow(int index ){
    return 1;
}
