#include "rdata.h"

void RData::Initialize(CUIForm _form, QAstra* _pqastra)
{
    // исключить дублирование при повторной инициализации
    this->clear();
    mCols_.clear();
    vCols_.clear();
    m_str_cols.clear();

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

    m_str_cols = "";
    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Type = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Type))->String()).Value();
        std::string col_Name = IRastrPayload(col->Name()).Value();
        std::string col_Title = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Title))->String()).Value();

        vCols_.push_back(col_Name);
        m_str_cols.append(col_Name);
        m_str_cols.append(",");

        RCol rc;
        rc.str_name_ = col_Name;
        rc.table_name_ = t_name_;
        rc.title_ = col_Title;
        rc.index = index;
        rc.setMeta(_pqastra);
        //Сначала все колонки добавляются с hidden=true,
        rc.hidden = true;

        int nRes = AddCol(rc);
        Q_ASSERT(nRes>=0);
        if (mCols_.find(col_Name) == mCols_.end())
            mCols_.insert(std::pair(col_Name,index));
    }

    //Скрыть колонки не входящие в форму
    //Скрытые колонки всё равно присутствуют в pnparray_
    for (CUIFormField &f : _form.Fields())
    {
        for  (RCol &rc : *this)
        {
            if (rc.str_name_ == f.Name())
                rc.hidden = false;
        }
    }

    if(m_str_cols.length()>0)
        m_str_cols.pop_back();
    qDebug() << "Open Table : " << t_name_.c_str();
    qDebug() << "Fields : " << m_str_cols.c_str();
}

std::string RData::getCommaSeparatedFieldNames(){
    std::string str_tmp;
    for( const RCol& col_data : *this ) {
        str_tmp += col_data.str_name_;
        str_tmp += ",";
    }
    if(str_tmp.length()>0){
        str_tmp.erase(str_tmp.length()-1);
    }
    return str_tmp;
}
void RData::Trace() const {
    for(const RCol& col : *this){
        qDebug() << " col: " << col.str_name_.c_str();
        for(const _col_data::value_type& cdata : col ){
            switch(col.en_data_){
            case RCol::_en_data::DATA_INT :
                qDebug()<<"cdata : "<< std::get<long>(cdata);
                break;
            case RCol::_en_data::DATA_DBL :
                qDebug()<<"cdata : "<< std::get<double>(cdata);
                break;
            case RCol::_en_data::DATA_STR :
                qDebug()<<"cdata : "<< std::get<std::string>(cdata).c_str();
                break;
            default:
                qDebug()<<"cdata : unknown!! ";
                break;
            }
            //qDebug()<<"cdata : "<< std::to_string(cdata).c_str();
        }
    }
}

void RData::populate_qastra(QAstra* _pqastra, RTablesDataManager* _pRTDM )
{
    /*
     * Идея иметь 1 хранилище данных таблицы для всех клиентов
     * Теперь попробуем обратиться к менеджеру данных таблиц
     * если такая таблица уже есть берем указатель на нее, если нет
     * тогда создаем в менеджере и отдаем указатель
    */
    pnparray_ = _pRTDM->get(t_name_, m_str_cols);
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
