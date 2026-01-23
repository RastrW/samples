#ifndef RTABLESDATAMANAGER_H
#define RTABLESDATAMANAGER_H
#pragma once

#include <map>
#include <string>
#include "QDataBlocks.h"
#include "QVector"
#include <QObject>
#include "qastra.h"
//#include "UIForms.h"
#include <astra/UIForms.h>

//class CUIForm;

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
    void setQAstra( QAstra* _pqastra);
    void SetForms ( std::list<CUIForm>* _lstUIForms);
    CUIForm* getForm ( std::string _name);
    void onRastrHint(const _hint_data& hint_data);
    std::shared_ptr<QDataBlock> Get(std::string tname, std::string Cols);

    long column_index(std::string tname, std::string _col_name);
    void GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB);
private:
    void GetDataBlock(std::string tname , std::string Cols , QDataBlock& QDB,FieldDataOptions Options );
    void GetDataBlock(std::string tname , QDataBlock& QDB,FieldDataOptions Options );
    void GetDataBlock(std::string tname , QDataBlock& QDB);
    std::string GetTCols(std::string tname);
    long GetColIndex(std::string tname,std::string cname);
    ePropType GetColType(std::string tname,std::string cname);


signals:
    void RTDM_dataChanged(std::string tname,int row_from , int col_from , int row_to, int col_to);
    void RTDM_UpdateModel(std::string tname);
    void RTDM_UpdateView(std::string tname);
    void RTDM_ResetModel(std::string tname);
    void RTDM_BeginResetModel(std::string tname);
    void RTDM_EndResetModel(std::string tname);
    void RTDM_BeginInsertRow(std::string tname,int first,int last);
    void RTDM_EndInsertRow(std::string tname);


private:
    QAstra* m_pqastra;
    std::list<CUIForm>* m_plstUIForms;

     /* Хранилище данных для моделей
      * 1:n то есть на 10 окон узлы -> 1 DataBlock
      * из overhead'а наверно только обновление данных если была открыта таблица , а потом
      * все её экземляры закрыты, так как удаления из хранилища пока нет.
      * */
    std::map<std::string,std::shared_ptr<QDataBlock>> mpTables;
};

#endif // RTABLESDATAMANAGER_H
