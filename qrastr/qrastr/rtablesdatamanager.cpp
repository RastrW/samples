#include "rtablesdatamanager.h"

//RTablesDataManager::RTablesDataManager() {}

void RTablesDataManager::setQAstra( QAstra* _pqastra)
{
    m_pqastra = _pqastra;
    connect(m_pqastra, &QAstra::onRastrHint, this, &RTablesDataManager::onRastrHint);
}
void  RTablesDataManager::SetForms ( std::list<CUIForm>* _lstUIForms)
{
    m_plstUIForms = _lstUIForms;
}
CUIForm*  RTablesDataManager::getForm ( std::string _name)
{
    for (CUIForm &form : *m_plstUIForms)
    {
        if (stringutils::cp1251ToUtf8(form.Name()) == _name)
            return &form;
    }
    return nullptr;
}
void  RTablesDataManager::onRastrHint(const _hint_data& hint_data)
{
    long row = hint_data.n_indx;
    std::string cname = hint_data.str_column;
    std::string tname = hint_data.str_table;

    std::map<std::string,std::shared_ptr<QDataBlock>>::iterator it;
    switch (hint_data.hint)
    {
    case EventHints::ChangeAll:
        for (auto [tname,sp_QDB] :  mpTables)
        {
            sp_QDB->Clear();
            GetDataBlock(tname,(*sp_QDB.get()));
            emit RTDM_UpdateModel(tname);
        }
        break;
    case EventHints::ChangeTable:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            //it->second->Clear();
            //GetDataBlock(tname,(*it->second.get()));
            //emit RTDM_UpdateModel(tname);

            // TO DO:
            // Вызвать чтение свойств столбцов
            // emit RTDM_UpdateProperties

        }
        break;
    case EventHints::ChangeData:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            FieldVariantData val = m_pqastra->GetVal(tname,cname,row);
            long ind = GetColIndex(tname,cname);
            it->second->Set(row,ind,val);
        }
        break;
    case EventHints::InsertRow:
    case EventHints::DeleteRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            std::string Cols = (*it).second->Columns();
            FieldDataOptions Options;
            Options.SetEnumAsInt(TriBool::True);
            Options.SetSuperEnumAsInt(TriBool::True);
            Options.SetUseChangedIndices(true);
            Options.SetEditatableColumnsOnly(true);                             // Без этого падает где то а разборке формул

            it->second->Clear();
            GetDataBlock(tname,Cols,(*it->second.get()),Options);
            emit RTDM_UpdateView(tname);
        }
        break;
        //case EventHints::DeleteRow:

    default:
        break;
    }
}

std::shared_ptr<QDataBlock>  RTablesDataManager::Get(std::string tname, std::string Cols)
{
    /*
     * Перед получением таблицы из RTDM удалим таблицы на которые никто не ссылается,
     * например если таблицу открыли, а потом закрыли, тогда она остается в RTDM со счетчиком ссылок (use_count()) = 1
     * если не удалять, тогда при расчете УР придется обновлять данные, но необходимости в этом нет.
    */
    //not work with map until < C++20
    //auto iter{ std::remove_if(begin(mpTables), end(mpTables), [&](auto& T) { return T.second.use_count() > 1; })};
    auto iter = mpTables.begin();
    auto endIter = mpTables.end();
    for(; iter != endIter; ) {
        if (iter->second.use_count() == 1) {
            qDebug()<<"RTDM: delete table with use_count() = 1 -> "<<iter->first;
            iter = mpTables.erase(iter);
        } else {
            ++iter;
        }
    }

    auto it = mpTables.find(tname);
    if (it != mpTables.end() )
    {
        return it->second;
        //it->second->Clear();
        //GetDataBlock(tname,(*it->second.get()));
    }
    else
    {
        mpTables.insert(std::make_pair(tname,new QDataBlock()));
        GetDataBlock(tname,Cols,*mpTables.find(tname)->second);
    }

    return mpTables.find(tname)->second;
}

void  RTablesDataManager::GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB,FieldDataOptions Options )
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::GetDataBlock(std::string tname , QDataBlock& QDB,FieldDataOptions Options )
{
    std::string Cols = GetTCols(tname);
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::GetDataBlock(std::string tname , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);
    GetDataBlock(tname,QDB,Options);
}

std::string  RTablesDataManager::GetTCols(std::string tname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload ColumnsCount{ columns->Count() };

    // Берем все колонки таблицы
    for (long index{ 0 }; index < ColumnsCount.Value(); index++)
    {
        IRastrColumnPtr col{ columns->Item(index) };
        std::string col_Type = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Type))->String()).Value();
        std::string col_Name = IRastrPayload(IRastrVariantPtr((col)->Property(FieldProperties::Name))->String()).Value();

        str_cols_.append(col_Name);
        str_cols_.append(",");
    }
    if(str_cols_.length()>0)
        str_cols_.pop_back();

    return str_cols_;
}
long  RTablesDataManager::GetColIndex(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    long res = IRastrPayload(col->Index()).Value();

    return res;
}
