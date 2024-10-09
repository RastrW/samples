#ifndef RTABLESDATAMANAGER_H
#define RTABLESDATAMANAGER_H
#include <map>
#include <string>
#include "QDataBlocks.h"
#include "QVector"
#include <QObject>
#include "qastra.h"

template<typename T>
class ITablesDataManagerT
{
    virtual void Add(std::string tname , QDenseDataBlock<T> DataBlock) = 0;
};


/* Класс для управления данными из метакита
 * Хранит данные по таблицам, в виджеты отдает указатели
 * обрабатывает события QAstra
*/
class RTablesDataManager : public QObject
{
    Q_OBJECT
public:
    void setQAstra( QAstra* _pqastra)
    {
         m_pqastra = _pqastra;
         connect(m_pqastra, &QAstra::onRastrHint, this, &RTablesDataManager::onRastrHint);
    }
    void onRastrHint(const _hint_data& hint_data)
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
                it->second->Clear();
                GetDataBlock(tname,(*it->second.get()));
                emit RTDM_UpdateModel(tname);
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

    std::shared_ptr<QDataBlock> Get(std::string tname, std::string Cols)
    {
        auto it = mpTables.find(tname);
        if (it != mpTables.end() )
        {
            //теперь это наверное лишнее , так как менеджер уже сам подписан на события
            //int cnt = it->second.use_count();
            //if (cnt == 1)                       // владеет только хранилище, виджетов нет -> зачит данные нужно перезапросить
            //{
                it->second->Clear();
                GetDataBlock(tname,(*it->second.get()));
            //}

            return mpTables.find(tname)->second;
        }
        else
        {
            mpTables.insert(std::make_pair(tname,new QDataBlock()));
            GetDataBlock(tname,Cols,*mpTables.find(tname)->second);

            return mpTables.find(tname)->second;
        }
    }

private:
    void GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB)
    {
        FieldDataOptions Options;
        Options.SetEnumAsInt(TriBool::True);
        Options.SetSuperEnumAsInt(TriBool::True);
        Options.SetUseChangedIndices(true);
        IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(tname) };
        IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
    }
    void GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB,FieldDataOptions Options )
    {
        IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(tname) };
        IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
    }
    void GetDataBlock(std::string tname , QDataBlock& QDB,FieldDataOptions Options )
    {
        std::string Cols = GetTCols(tname);
        IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(tname) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrResultVerify(table->DataBlock(Cols, QDB, Options));
    }
    void GetDataBlock(std::string tname , QDataBlock& QDB)
    {
        FieldDataOptions Options;
        Options.SetEnumAsInt(TriBool::True);
        Options.SetSuperEnumAsInt(TriBool::True);
        Options.SetUseChangedIndices(true);
        GetDataBlock(tname,QDB,Options);
    }

    std::string GetTCols(std::string tname)
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
    long GetColIndex(std::string tname,std::string cname)
    {
        std::string str_cols_ = "";
        IRastrTablesPtr tablesx{ m_pqastra->getRastr()->Tables() };
        IRastrTablePtr table{ tablesx->Item(tname) };
        IRastrColumnsPtr columns{ table->Columns() };
        IRastrColumnPtr col{ columns->Item(cname) };
        long res = IRastrPayload(col->Index()).Value();

        return res;
    }

signals:
    void RTDM_UpdateModel(std::string tname);
    void RTDM_UpdateView(std::string tname);

private:
    QAstra* m_pqastra;
     /* Хранилище данных для моделей
      * 1:n то есть на 10 окон узлы -> 1 DataBlock
      * из overhead'а наверно только обновление данных если была открыта таблица , а потом
      * все её экземляры закрыты, так как удаления из хранилища пока нет.
      * */
    std::map<std::string,std::shared_ptr<QDataBlock>> mpTables;
};

#endif // RTABLESDATAMANAGER_H
