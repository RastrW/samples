#pragma once

#include <QWidget>
#include <QString>
#include <string>
#include <unordered_map>
#include <memory>
#include "UIForms.h"
#include "QtitanGrid.h"

class QAstra;
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

class RTablesDataManager;
namespace ads { class CDockManager; }
namespace Qtitan { class GridTableView; }

/**
 * @brief Контроллер таблицы Rastr — управляет данными и
 * отображает одну таблицу Rastr в QTitan Grid.
 *
 * Наследует QWidget по двум причинам:
 *   1. Родитель для диалогов (ColPropDialog, GroupCorrectionDialog и др.),
 *      которым нужен QWidget* parent.
 *   2. Реализует closeEvent для корректного отключения сигналов.
 *
 * Сам невидим: dw->setWidget(controller->createShell()), а не сам контроллер.
 */
class RtabController : public QWidget
{
    Q_OBJECT
public:
    explicit RtabController(QAstra*             pqastra,
                            CUIForm             UIForm,
                            RTablesDataManager* pRTDM,
                            ads::CDockManager*  pDockManager,
                            QWidget*            parent = nullptr);

    ~RtabController() override;
    //отключает сигналы LinkedForm, сбрасывает pnparray_.
    void closeEvent(QCloseEvent* event) override;

    /**
     * @brief Создаёт и возвращает RtabShell — видимую оболочку для CDockWidget.
     * Можно вызвать только один раз: повторный вызов вернёт nullptr и залогирует ошибку.
     * @param withToolbar  false для связанных форм (без тулбара и шорткатов).
     */
    RtabShell* createShell(bool withToolbar = true);

    // ── Public API (данные) ──────────────────────────────────────────────────
    int  getLongValue(const std::string& key, long row);
    /// @brief Применяет LinkedForm через контроллер.
    void applyLinkedFormFromController(const LinkedForm& lf);
    void notifyParentRowChanged(int modelRow);
    void setPyHlp(std::shared_ptr<PyHlp> pPyHlp);

    /// Регистрирует шорткаты Ctrl+I/A/R/D на grid.
    /// Вызывается из RtabShell (только когда withToolbar = true).
    static void setupShortcuts(RtabController* target, RGrid* grid);

public slots:
    void slot_close();

    // ── Строки ──────────────────────────────────────────────────────────────
    void slot_addRow();
    void slot_insertRow();
    void slot_duplicateRow();
    void slot_deleteRow();
    void slot_groupCorrection();

    // ── Колонки ─────────────────────────────────────────────────────────────
    void slot_directCodeToggle(std::size_t column);
    void slot_condFormatsEdit(std::size_t column);
     //  Формы инструментов
    void slot_openColProp(int col);
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
    void slot_beginResetModel(std::string tname);
    void slot_endResetModel(std::string tname);

private:
    /** @brief
     * a) создаёт RModel, вызывает setForm/populateDataFromRastr;
     * b) подключает сигналы RTDM к слотам RModel (обновления данных);
     * c) устанавливает редакторы колонок (SetEditors);
     * d) восстанавливает условное форматирование из JSON.
    */
    void createModel(QAstra* pqastra);
    void applyAllColumnEditors();
    void applyColumnEditor(int colIndex);
    void setTableView(int multiplier = 10);
    void setupConnections();

    // ── Компоненты (данные) ─────────────────────────────────────────────────
    std::unique_ptr<RModel>               m_model;
    std::unique_ptr<FilterManager>        m_filterManager;
    std::unique_ptr<LinkedFormController> m_linkedFormCtrl;
    std::unique_ptr<ContextMenuBuilder>   m_menuBuilder;
    std::unique_ptr<CondFormatController> m_condFormatCtrl;

    // ── Грид (создан здесь, передан в RtabShell) ────────────────────────────
    RGrid*                 m_grid  {nullptr};
    Qtitan::GridTableView* m_view  {nullptr};

    // ── Конфигурация ────────────────────────────────────────────────────────
    CUIForm              m_UIForm;
    RTablesDataManager*  m_pRTDM       {nullptr};
    ads::CDockManager*   m_DockManager {nullptr};
    std::shared_ptr<PyHlp> pPyHlp_;

    std::unordered_map<QString, bool> m_columnsVisible;

    /// Защита от повторного вызова createShell()
    bool m_shellCreated {false};
};
