#pragma once
#include <memory>
#include <QObject>
#include "astra/IPlainRastr.h"
#include "ITableRepository.h"
#include "ITableEvents.h"

class QAstra;
class CUIForm;
class QDataBlock;
class _hint_data;

/**
 * @class Централизованный кеш табличных данных и маршрутизатор событий от расчётного ядра.
 *
 * Ответственности:
 *  1. Хранит кеш mpTables: map<имя_таблицы, shared_ptr<QDataBlock>>.
 *     Один QDataBlock на таблицу, независимо от числа открытых окон.
 *  2. Слушает сигнал QAstra::onRastrHint и преобразует hint-события плагина
 *     в Qt-сигналы (sig_dataChanged, sig_BeginResetModel, …).
 *  3. RModel подписывается на эти сигналы и уведомляет View об изменениях.
 *  4. Является единственной точкой записи в плагин через setValue().
 *
 * Время жизни QDataBlock:
 *   Блок удаляется из кеша при следующем вызове get(), если use_count() == 1.
 */
class RTablesDataManager : public ITableEvents,
                           public ITableRepository
{
    Q_OBJECT
public:
    RTablesDataManager(std::shared_ptr<QAstra> _pqastra);
    void setForms ( std::list<CUIForm>* _lstUIForms);
    ///< Обработчик событий от Rastr
    void onRastrHint(const _hint_data& hint_data);

    virtual TableSchema getSchema(const std::string& tname) override;
    // ── Данные ───────────────────────────────────────────────────────────────
    /**
     * Получить (или создать) QDataBlock для таблицы tname.
     *
     * Побочный эффект: перед поиском очищает "мёртвые" блоки (use_count == 1).
     * Это дешевле, чем хранить неиспользуемые данные при пересчётах.
     * @param tname  имя таблицы плагина (например "node", "vetv")
     * @param Cols   строка колонок через запятую ("ny,name,uhom")
     */
    std::shared_ptr<QDataBlock>
    getBlock(const std::string& tname,
             const std::string& cols) override;
    void fillBlock(const std::string& tname,
                   QDataBlock&        block,
                   const std::string& cols = "",
                   std::optional<FieldDataOptions> opts = std::nullopt) override;
    long columnIndex(const std::string& tname,
                     const std::string& colName) override;
    /// Применяет выборку к таблице tname и возвращает вектор строковых индексов.
    std::vector<long>
    rowsBySelection(const std::string& tname,
                    const std::string& selection) override;
    // ── Запись ───────────────────────────────────────────────────────────────
    /** @brief Централизованная запись в плагин.
    * RModel::setData больше НЕ вызывает emit dataChanged вручную —
    * обновление View приходит ровно один раз, через цепочку хинтов.
    */
    void setValue(const std::string&      tname,
                  const std::string&      colName,
                  long                    row,
                  const FieldVariantData& value) override;

    void setColumnProperty(const std::string& tname,
                                   const std::string& colName,
                                   FieldProperties    prop,
                                   const std::string& value) override;

    void calcColumn(const std::string& tname,
                            const std::string& colName,
                            const std::string& expression,
                            const std::string& selection) override;

    void addRows   (const std::string& tname,
                size_t             count) override;
    void insertRows(const std::string& tname,
                   long               startRow,
                   int                count) override;
    void deleteRows(const std::string& tname,
                   long               startRow,
                   int                count) override;
    void duplicateRow(const std::string& tname, long row) override;

    long tableSize(const std::string& tname) override;
    void setTableSize(const std::string& tname, long size) override;
    //──Администрирование ───────────────────────────────────────────────
    void exportToCsv(const std::string& tname,
                     const std::string& cols,
                     const std::string& selection,
                     const std::string& path,
                     const std::string& divider,
                     eCSVCode           mode) override;
    void importToCsv(const std::string& tname,
                     const std::string& cols,
                     const std::string& selection,
                     const std::string& file,
                     const std::string& divider,
                     const std::string& byDefault,
                     eCSVCode           mode) override;
    void setLockEvent(bool lock) override;
    CUIForm* getForm (const std::string& name) override;
private:
    std::shared_ptr<QAstra> m_pqastra;
    std::list<CUIForm>* m_plstUIForms;

     /* Хранилище данных для моделей
      * 1:n то есть на 10 окон узлы -> 1 DataBlock
      * из overhead'а наверно только обновление данных если была открыта таблица , а потом
      * все её экземляры закрыты, так как удаления из хранилища пока нет.
      * */
    std::unordered_map<std::string,std::shared_ptr<QDataBlock>> mpTables;

    // =========================================================================
    //  Обработчики отдельных типов событий
    // =========================================================================
    /**
     * @brief EventHints::ChangeAll — все таблицы изменились (например, загрузка файла).
     * Сбрасывает и перечитывает каждый кешированный блок.
     * @note Вызыывается при:
     *     - writeTable
     *     - Table::SetSize
     *     - Table::SwapRows
     *     - Table::AddTable - создание новой таблицы через параметры
     *     - Column::SetProperty - изменение свойств поля
     *     - Columns::RemoveAll
     *     - Columns::Remove
     *     - CalcRgm
     */
    void handleChangeAll();
    /**
     * @brief EventHints::ChangeTable — изменилась схема таблицы (добавлена/удалена колонка).
     * Полная перезагрузка блока включая структуру колонок.
     */
    void handleChangeTable(const std::string& tname);
    /**
     * @brief EventHints::ChangeColumn — изменились все значения одной колонки.
     * Перечитывает только указанную колонку построчно.
     */
    void handleChangeColumn(const std::string& tname, const std::string& cname);
    /**
     * @brief EventHints::ChangeRow — изменились все колонки одной строки.
     * Перечитывает все колонки для строки row.
     * Изменились все значения одной колонки:
     * - групповой коррекция
     * - при изменении содержимого одной ячейки может быть вызван при наличии связанной формулы в колонке
     */
    void handleChangeRow(const std::string& tname, long row);
    /**
     * @brief EventHints::ChangeData — изменилось одно значение (tname, cname, row).
     * При изменении в ячейке должна изменяться вся строка,
     * т.к. значения в колонках могут быть связаны между собой
     */
    void handleChangeData(const std::string& tname,
                          const std::string& cname,
                          long row);
    /// @brief EventHints::AddRow — новая строка добавлена в конец таблицы. */
    void handleAddRow(const std::string& tname, long row);
    /// @brief EventHints::InsertRow — строка вставлена в позицию row. */
    void handleInsertRow(const std::string& tname, long row);
    /// @brief EventHints::DeleteRow — строка row удалена. */
    void handleDeleteRow(const std::string& tname, long row);

    // =========================================================================
    //  Вспомогательные методы доступа к плагину
    // =========================================================================
    /// @brief Найти кешированный блок или вернуть nullptr
    QDataBlock* findCachedBlock(const std::string& tname);

    std::string getTCols(const std::string& tname);
    long getColIndex(const std::string& tname,
                     const std::string& cname);
};
