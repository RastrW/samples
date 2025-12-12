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
        //if (stringutils::cp1251ToUtf8(form.Name()) == _name)
        if (stringutils::MkToUtf8(form.Name()) == _name)
            return &form;
    }
    return nullptr;
}
void  RTablesDataManager::onRastrHint(const _hint_data& hint_data)
{
    long row = hint_data.n_indx;
    std::string cname = hint_data.str_column;
    std::string tname = hint_data.str_table;
    qDebug() << "Hint: type: " << (long)(hint_data.hint) << " tname: " <<tname << " col_name: " << cname;

    std::map<std::string,std::shared_ptr<QDataBlock>>::iterator it;
    switch (hint_data.hint)
    {
    case EventHints::ChangeAll:
        for (auto [tname,sp_QDB] :  mpTables)
        {
            emit RTDM_BeginResetModel(tname);
            sp_QDB->Clear();
            GetDataBlock(tname,(*sp_QDB.get()));
            emit RTDM_EndResetModel(tname);
        }
        break;
    case EventHints::ChangeTable:
        emit RTDM_BeginResetModel(tname);
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
        emit RTDM_EndResetModel(tname);
        break;
    case EventHints::ChangeColumn:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit RTDM_BeginResetModel(tname);
            IRastrTablesPtr tables{ m_pqastra->getRastr()->Tables() };
            IRastrTablePtr table{ tables->Item(tname) };
            DataBlock<FieldVariantData> variant_block;
            IRastrResultVerify(table->DataBlock(cname, variant_block));

            long col_ind = GetColIndex(tname,cname);
            ePropType col_type = GetColType(tname,cname);
            for (size_t i = 0; i < (*it).second->RowsCount() ; i++)
            {
                switch (col_type)
                {
                    case ePropType::Double:
                        it->second->Set(i,col_ind,  std::visit(ToDouble(),variant_block.Get(i,0)));
                    break;
                    case ePropType::Int:
                    case ePropType::Enum:
                    case ePropType::Superenum:
                    case ePropType::Enpic:
                    case ePropType::Color:
                        it->second->Set(i,col_ind,  std::visit(ToLong(),variant_block.Get(i,0)));
                        break;
                    case ePropType::String:
                        it->second->Set(i,col_ind,  std::visit(ToString(),variant_block.Get(i,0)));
                        break;
                    default:
                        break;
                }
            }
            emit RTDM_EndResetModel(tname);
        }
        break;

    case EventHints::ChangeRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            //emit RTDM_BeginResetModel(tname);

            QDataBlock* pqdb = it->second.get();
            IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
            IRastrPayload tablecount{ tablesx->Count() };
            IRastrTablePtr table{ tablesx->Item(tname) };
            IRastrObjectPtr<IPlainRastrColumns> columns{ table->Columns() };
            IRastrPayload columnscount{columns->Count()};
            for (int i = 0 ; i < columnscount.Value(); i++)
            {
                IRastrColumnPtr col {columns->Item(i)};
                IRastrVariantPtr value_ptr{ col->Value(row) };
                pqdb->Set(row,i,value_ptr);
            }
            emit RTDM_dataChanged(tname,row,0,row,columnscount.Value());
           // emit RTDM_EndResetModel(tname);
        }
        break;

    case EventHints::ChangeData:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            //emit RTDM_BeginResetModel(tname);
            QDataBlock* pqdb = it->second.get();
            long col_ind = GetColIndex(tname,cname);

            IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
            IRastrPayload tablecount{ tablesx->Count() };
            IRastrTablePtr table{ tablesx->Item(tname) };
            IRastrObjectPtr<IPlainRastrColumns> columns{ table->Columns() };
            IRastrPayload rowscount{table->Size()};
            IRastrColumnPtr col {columns->Item(cname)};
            IRastrVariantPtr v_ptr{ col->Value(row) };
            IRastrPayload columnscount{columns->Count()};
            long cols_cnt = columnscount.Value();
            long rows_cnt = rowscount.Value();

            long pnparr_nrows = it->second->RowsCount();
            long pnparr_ncols = it->second->ColumnsCount();

            if (col_ind >= pnparr_ncols )
            {
                qDebug()<<"EventHints::ChangeData: col_ind > cols_cnt " <<col_ind << ">" <<cols_cnt;
                break;
            }
            if (row >= pnparr_nrows )
            {
                qDebug()<<"EventHints::ChangeData: row > rows_cnt " <<row << ">" <<rows_cnt;
                break;
            }

            //Было
            //FieldVariantData val = m_pqastra->GetVal(tname,cname,row);
            //pqdb->Set(row,col_ind,val);

            //Стало: обновляем всю строку
            // ERROR: При добавлении строки в таблицу узлы и при попытке получить это строку в VDB падает где то в разюоршике формул
            DataBlock<FieldVariantData> VDB;
            VDB.IndexesVector() = {row};
            try {
                IRastrResultVerify(table->DataBlock("", VDB));
            }
            catch (...) {
                FieldDataOptions Options;
                Options.SetEnumAsInt(TriBool::True);
                Options.SetSuperEnumAsInt(TriBool::True);
                Options.SetEditatableColumnsOnly(true);
                IRastrResultVerify(table->DataBlock("", VDB, Options));
            }

            //VDB.QDump();

            for (int i = 0 ; i < columnscount.Value(); i++)
            {
                size_t ind = row * columnscount.Value() + i;
                size_t ind_vdb = 0 * columnscount.Value() + i;
                pqdb->Data()[ind] = VDB.Data()[ind_vdb];
            }
           // emit RTDM_dataChanged(tname,row,0,row,cols_cnt);
            //emit RTDM_EndResetModel(tname);
        }
        break;

    case EventHints::AddRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit RTDM_BeginInsertRow(tname,row,row);
            QDataBlock* pqdb = it->second.get();
            pqdb->AddRow();
            emit RTDM_EndInsertRow(tname);
        }
        break;

    case EventHints::InsertRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit RTDM_BeginInsertRow(tname,row,row);
            QDataBlock* pqdb = it->second.get();
            pqdb->InsertRow(row);
            emit RTDM_EndInsertRow(tname);
            //pqdb->QDump(10,20);
        }
        break;
    case EventHints::DeleteRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit RTDM_BeginResetModel(tname);
            QDataBlock* pqdb = it->second.get();
            pqdb->DeleteRow(row);
            emit RTDM_EndResetModel(tname);
        }
        break;


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
            qDebug()<<"RTDM: delete table with use_count() = 1 -> " <<iter->first.c_str();
            iter = mpTables.erase(iter);
        } else {
            qDebug()<<"RTDM: " << iter->first.c_str() <<" use_count() =  " <<iter->second.use_count();
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
        qDebug()<<"RTDM: add Table" << tname;
        mpTables.insert(std::make_pair(tname,new QDataBlock()));
        GetDataBlock(tname,Cols,*mpTables.find(tname)->second);
    }

    return mpTables.find(tname)->second;
}

long RTablesDataManager::column_index(std::string tname , std::string _col_name)
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrPayload tindex{tablesx->FindIndex(tname)};
    if ( tindex.Value() < 0 )
        return -1;
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrPayload res{columns->FindIndex(_col_name)};
    return res.Value();
}
void  RTablesDataManager::GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);
    //Options.SetEditatableColumnsOnly(true);
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
    //Options.SetEditatableColumnsOnly(true);
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
ePropType  RTablesDataManager::GetColType(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    ePropType res = IRastrPayload(col->Type()).Value();

    return res;
}
