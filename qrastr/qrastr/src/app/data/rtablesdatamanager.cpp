#include "rtablesdatamanager.h"

//RTablesDataManager::RTablesDataManager() {}

void RTablesDataManager::setQAstra( QAstra* _pqastra)
{
    m_pqastra = _pqastra;
    connect(m_pqastra, &QAstra::onRastrHint, this, &RTablesDataManager::onRastrHint);
}

void  RTablesDataManager::setForms ( std::list<CUIForm>* _lstUIForms)
{
    m_plstUIForms = _lstUIForms;
}

CUIForm*  RTablesDataManager::getForm ( std::string _name)
{
    for (CUIForm &form : *m_plstUIForms)
    {
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
    qDebug() << "Hint: type: " << (long)(hint_data.hint) << " tname: " <<tname.c_str() << " col_name: " << cname.c_str();

    std::map<std::string,std::shared_ptr<QDataBlock>>::iterator it;
    switch (hint_data.hint)
    {
    case EventHints::ChangeAll:
        for (auto [tname,sp_QDB] :  mpTables)
        {
            emit sig_BeginResetModel(tname);
            sp_QDB->Clear();
            getDataBlock(tname,(*sp_QDB.get()));
            emit sig_EndResetModel(tname);
        }
        break;
    case EventHints::ChangeTable:
        emit sig_BeginResetModel(tname);
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
        emit sig_EndResetModel(tname);
        break;
    case EventHints::ChangeColumn:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit sig_BeginResetModel(tname);
            IRastrTablesPtr tables{ m_pqastra->getRastr()->Tables() };
            IRastrTablePtr table{ tables->Item(tname) };
            DataBlock<FieldVariantData> variant_block;
            IRastrResultVerify(table->DataBlock(cname, variant_block));

            long col_ind = getColIndex(tname,cname);
            ePropType col_type = getColType(tname,cname);
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
            emit sig_EndResetModel(tname);
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
            emit sig_dataChanged(tname,row,0,row,columnscount.Value());
           // emit RTDM_EndResetModel(tname);
        }
        break;

    case EventHints::ChangeData:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            //emit RTDM_BeginResetModel(tname);
            QDataBlock* pqdb = it->second.get();
            long col_ind = getColIndex(tname,cname);

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
            for (int i = 0 ; i < columnscount.Value(); i++)
            {
                size_t ind = row * columnscount.Value() + i;
                size_t ind_vdb = 0 * columnscount.Value() + i;
                pqdb->Data()[ind] = VDB.Data()[ind_vdb];
            }
        }
        break;

    case EventHints::AddRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit sig_BeginInsertRow(tname,row,row);
            QDataBlock* pqdb = it->second.get();
            pqdb->AddRow();
            emit sig_EndInsertRow(tname);
        }
        break;

    case EventHints::InsertRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit sig_BeginInsertRow(tname,row,row);
            QDataBlock* pqdb = it->second.get();
            pqdb->InsertRow(row);
            emit sig_EndInsertRow(tname);
        }
        break;
    case EventHints::DeleteRow:
        it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            emit sig_BeginResetModel(tname);
            QDataBlock* pqdb = it->second.get();
            pqdb->DeleteRow(row);
            emit sig_EndResetModel(tname);
        }
        break;

    default:
        break;
    }
}

std::shared_ptr<QDataBlock>  RTablesDataManager::get(std::string tname, std::string Cols)
{
    /*
     * Перед получением таблицы из RTDM удалим таблицы на которые никто не ссылается,
     * например если таблицу открыли, а потом закрыли, тогда она остается в RTDM со счетчиком ссылок (use_count()) = 1
     * если не удалять, тогда при расчете УР придется обновлять данные, но необходимости в этом нет.
    */
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
    }else{
        qDebug()<<"RTDM: add Table" << tname.c_str();
        mpTables.insert(std::make_pair(tname, std::make_shared<QDataBlock>()));
        getDataBlock(tname,Cols,*mpTables.find(tname)->second);
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
void  RTablesDataManager::getDataBlock(std::string tname , std::string Cols , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);

    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };

    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::getDataBlock(std::string tname , std::string Cols , QDataBlock& QDB,FieldDataOptions Options )
{
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::getDataBlock(std::string tname , QDataBlock& QDB,FieldDataOptions Options )
{
    std::string Cols = getTCols(tname);
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
}
void  RTablesDataManager::getDataBlock(std::string tname , QDataBlock& QDB)
{
    FieldDataOptions Options;
    Options.SetEnumAsInt(TriBool::True);
    Options.SetSuperEnumAsInt(TriBool::True);
    Options.SetUseChangedIndices(true);

    getDataBlock(tname,QDB,Options);
}

std::string  RTablesDataManager::getTCols(std::string tname)
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
long  RTablesDataManager::getColIndex(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    long res = IRastrPayload(col->Index()).Value();

    return res;
}
ePropType  RTablesDataManager::getColType(std::string tname,std::string cname)
{
    std::string str_cols_ = "";
    IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(tname) };
    IRastrColumnsPtr columns{ table->Columns() };
    IRastrColumnPtr col{ columns->Item(cname) };
    ePropType res = IRastrPayload(col->Type()).Value();

    return res;
}
