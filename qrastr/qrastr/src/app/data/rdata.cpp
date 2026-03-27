#include "rdata.h"
#include "rtablesdatamanager.h"
#include "astra_headers/UIForms.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "qastra.h"

RData::RData(QAstra* _pqastra, const CUIForm& _form):
    m_qastra{_pqastra}{

    t_name_ = _form.TableName();

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
    spdlog::debug("ColumnsCount : {}", ColumnsCount.Value());
    reserve( n_reserve);                // Без reserve RCol данные обнуляются видимио при reallocation  If a reallocation happens, all contained elements are modified.
    spdlog::debug("reserve : {} ok", n_reserve);

    m_str_cols = "";
    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Name = IRastrPayload(col->Name()).Value();

        vCols_.push_back(col_Name);
        m_str_cols.append(col_Name);
        m_str_cols.append(",");

        RCol rc;
        //Сначала все колонки добавляются с hidden=true,
        rc.initialize(col_Name, t_name_, index);
        rc.setMeta(_pqastra);

        int nRes = AddCol(rc);
        Q_ASSERT(nRes>=0);
        if (mCols_.find(col_Name) == mCols_.end())
            mCols_.insert(std::pair(col_Name,index));
    }

    //Скрыть колонки не входящие в форму
    //Скрытые колонки всё равно присутствуют в pnparray_
    for (const CUIFormField &f : _form.Fields())
    {
        for  (RCol &rc : *this)
        {
            if (rc.getColName() == f.Name())
                rc.setHidden(false);
        }
    }

    if(m_str_cols.length()>0)
        m_str_cols.pop_back();
    spdlog::debug("Open Table : {}", t_name_.c_str());
    spdlog::debug("Fields : {}", m_str_cols.c_str());
}

QAstra* RData::getAstra() const { return m_qastra; }

int RData::AddCol(const RCol& rcol){
    emplace_back(rcol);
    return static_cast<int>(size());
}

std::string RData::getCommaSeparatedFieldNames(){
    std::string str_tmp;
    for( const RCol& col_data : *this ) {
        str_tmp += col_data.getColName();
        str_tmp += ",";
    }
    if(str_tmp.length()>0){
        str_tmp.erase(str_tmp.length()-1);
    }
    return str_tmp;
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
            if (!rc.isHidden())
                ret_cols += rc.getColName() + ",";
    }
    else
    {
        for  (RCol &rc : *this)
            ret_cols += rc.getColName() + ",";
    }

    if (!ret_cols.empty())
        ret_cols.pop_back();

    return ret_cols;
}
