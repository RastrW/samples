#pragma once

#include <QWidget>
#include <QString>
#include <string>
#include <unordered_map>
#include <memory>
#include "UIForms.h"
#include "QtitanGrid.h"
#include "table/tableIndexTypes.h"

class ITableRepository;
class ITableEvents;
class RGrid;
class RModel;
class RCol;
class FilterManager;
class RtabShell;
class LinkedFormController;
class ContextMenuBuilder;
class CondFormatController;
class PyHlp;
class LinkedForm;
struct FilterRule;
class TableDockManager;
class RData;

namespace ads { class CDockManager; }
namespace Qtitan { class GridTableView; }

struct TableProperties{
    bool isLinked {false};
    bool isVertical {false};
    bool withToolbar {true};
    QString formQName;
};

/// @brief Контроллер таблицы Rastr — управляет данными и
/// отображает одну таблицу Rastr в QTitan Grid.
class RtabController : public QObject
{
    Q_OBJECT
public:
    explicit RtabController(std::shared_ptr<ITableRepository> tables,
                            std::shared_ptr<ITableEvents>     tableEvents,
                            CUIForm             UIForm,
                            ads::CDockManager*  pDockManager,
                            TableDockManager*   tableDockManager,
                            QObject*            parent = nullptr);
    ~RtabController() override;

    struct CommonTableActions {
        QAction* addRow       = nullptr;
        QAction* insertRow    = nullptr;
        QAction* deleteRow    = nullptr;
        QAction* duplicateRow = nullptr;
        QAction* groupCorr    = nullptr;
    };
    /**
     * @brief Создаёт и возвращает RtabShell — видимую оболочку для CDockWidget.
     * Можно вызвать только один раз: повторный вызов вернёт nullptr и залогирует ошибку.
     * @param withToolbar  false для связанных форм (без тулбара и шорткатов).
     */
    RtabShell* createShell(const TableProperties& tabProp);

    // ── Public API (данные) ──────────────────────────────────────────────────
    int  getLongValue(const std::string& key, long row);
    /// @brief Применяет LinkedForm через контроллер.
    void applyLinkedFormFromController(const LinkedForm& lf);
    void notifyParentRowChanged(int modelRow);
    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);

    /// Регистрирует шорткаты Ctrl+I/A/R/D на grid.
    /// Вызывается только когда withToolbar = true.
    void setupShortcuts(RGrid* grid);
    const CommonTableActions& actions() const { return m_comTabAct; }

    /// Снять фильтр связанной формы (вызывается при закрытии родительской таблицы)
    void clearLinkedFilter();
public slots:
    // ── Строки ──────────────────────────────────────────────────────────────
    void slot_addRow();
    void slot_insertRow();
    void slot_duplicateRow();
    void slot_deleteRow();
    void slot_groupCorrection();

    // ── Колонки ─────────────────────────────────────────────────────────────
    void slot_directCodeToggle(ModelIndex column);
    void slot_condFormatsEdit(ModelIndex column);
     //  Формы инструментов
    void slot_openColProp(ModelIndex col);
    // ширина по шаблону
    void slot_widthByTemplate();
    // ширина по контенту
    void slot_widthByData();

    // ── Фильтр ──────────────────────────────────────────────────────────────
    void slot_toggleAutoFilter(bool checked);
    void slot_applyAutoFilter(int colIndex, const FilterRule& rule);
    void slot_setFiltrForSelection(std::string selection);

    // ── Импорт / экспорт ────────────────────────────────────────────────────
    void slot_openExportCSVForm();
    void slot_openImportCSVForm();

private slots:
    void slot_contextMenu(ContextMenuEventArgs* args);
    void slot_contextMenuVertical(ContextMenuEventArgs* args);
    void slot_beginResetModel(const std::string& tname);
    void slot_endResetModel(const std::string& tname);

private:
    /** @brief
     * a) создаёт RModel, вызывает setForm/populateDataFromRastr;
     * b) подключает сигналы RTDA к слотам RModel (обновления данных);
     * c) устанавливает редакторы колонок (SetEditors);
     * d) восстанавливает условное форматирование из JSON.
    */
    void createModel(std::shared_ptr<ITableRepository> tables);
    void applyAllColumnEditors();
    /** @note QTitan берёт ownership переданного репозитория:
     * GridColumn::setEditorRepository удаляет предыдущий объект перед заменой,
     * GridColumn::~GridColumn удаляет текущий при уничтожении колонки.
     * Повторный вызов applyColumnEditor безопасен — утечки нет.
    */
    void applyColumnEditor(ModelIndex col);
    void setTableView(bool update = true, int multiplier = 10);
    void setupConnections();
    void createCommonTableActions();
    void restoreColumnOrder(const RData& rdata);
    void restoreColumnVisibility(const RData& rdata);

    void setupColumnOrder();
    void createLinkedFormController(std::shared_ptr<ITableRepository> tables);
    void createCondFormatController();
    std::string getVisibleColsFromGrid() const;
    /// Возвращает позицию колонки в RData по имени, или -1.
    ModelIndex modelIndexOf(const std::string& colName) const;
    //колонка → binding → ModelIndex
    ModelIndex modelIndexFromListIndex(int listIdx) const;

    Qtitan::GridTableColumn* columnByModelIndex(ModelIndex pos)const;
    // ── Компоненты (данные) ─────────────────────────────────────────────────
    std::unique_ptr<RModel>               m_model;
    std::unique_ptr<FilterManager>        m_filterManager;
    std::unique_ptr<LinkedFormController> m_linkedFormCtrl;
    std::unique_ptr<ContextMenuBuilder>   m_menuBuilder;
    std::unique_ptr<CondFormatController> m_condFormatCtrl;
    std::shared_ptr<ITableRepository>     m_tables;
    std::shared_ptr<ITableEvents>     m_tableEvents;
    // ── Грид (создан здесь, передан в RtabShell) ────────────────────────────
    RGrid*                 m_grid  {nullptr};
    Qtitan::GridTableView* m_view  {nullptr};

    // ── Конфигурация ────────────────────────────────────────────────────────
    CUIForm              m_UIForm;
    ads::CDockManager*   m_DockManager {nullptr};
    struct ColumnState {
        bool      visible  = true;
        VisualIndex vi     = VisualIndex{-1}; // -1 = не задан
    };
    std::unordered_map<QString, ColumnState> m_columnState;
    /// Защита от повторного вызова createShell()
    bool m_shellCreated {false};
    CommonTableActions m_comTabAct;
    TableDockManager* m_tableDockManager;
};
