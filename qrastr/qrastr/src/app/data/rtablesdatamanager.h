#pragma once

#include <map>
#include <string>
#include "QDataBlocks.h"
#include "QVector"
#include <QObject>
#include "qastra.h"
#include "astra_headers/UIForms.h"

/* Класс для управления данными из метакита
 * - Централизованное хранение данных таблиц
 * - Обработка событий от QAstra (EventHints)
 * - Управление жизненным циклом DataBlock
 * - Синхронизация данных между расчётным ядром и UI
 * - Реализация паттерна "1 DataBlock : N Views"
*/
class RTablesDataManager : public QObject
{
    Q_OBJECT
public:
    void setQAstra( QAstra* _pqastra);
    void setForms ( std::list<CUIForm>* _lstUIForms);
    CUIForm* getForm ( std::string _name);
    ///< Обработчик событий от Rastr
    void onRastrHint(const _hint_data& hint_data);
    ///<Получение/создание DataBlock
    std::shared_ptr<QDataBlock>
        get(std::string tname, std::string Cols);

    long column_index(std::string tname, std::string _col_name);
    void getDataBlock(std::string tname, std::string Cols, QDataBlock& QDB);
private:
    void getDataBlock(std::string tname, std::string Cols, QDataBlock& QDB, FieldDataOptions Options );
    void getDataBlock(std::string tname, QDataBlock& QDB, FieldDataOptions Options );
    void getDataBlock(std::string tname, QDataBlock& QDB);
    std::string getTCols(std::string tname);
    long getColIndex(std::string tname,std::string cname);
    ePropType getColType(std::string tname,std::string cname);
signals:
    ///< изменены данные в диапазоне
    void sig_dataChanged(std::string tname,int row_from ,
                         int col_from , int row_to, int col_to);
    ///< перестроение модели
    void sig_BeginResetModel(std::string tname);
    void sig_EndResetModel(std::string tname);
    ///< вставка строки
    void sig_BeginInsertRow(std::string tname,int first,int last);
    void sig_EndInsertRow(std::string tname);
    ///< обновление представлений
    void sig_UpdateModel(std::string tname);
    void sig_UpdateView(std::string tname);
    void sig_ResetModel(std::string tname);
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
