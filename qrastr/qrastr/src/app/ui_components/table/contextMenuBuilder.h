#pragma once
#include <QObject>
#include "rtabcontroller.h"

class RCol;
class LinkedFormController;
class QMenu;
class QAction;

namespace Qtitan{ class GridTableView; }

/// Контекст одного показа меню.
struct MenuContext {
    ModelColumn column;
    int   row    = -1;
    const RCol* col    = nullptr;   ///< указатель действителен на время показа меню
};

class ContextMenuBuilder : public QObject
{
    Q_OBJECT
public:
    explicit ContextMenuBuilder(Qtitan::GridTableView*   view,
                                LinkedFormController*    linkedFormCtrl,
                                const RtabController::CommonTableActions& actions,
                                QObject*                 parent = nullptr);

    QAction* actionExport() const { return m_actExport; }
    QAction* actionImport() const { return m_actImport; }

    /// Строит персистентный QMenu и все статичные QAction.
    void initMenu(QWidget* menuParent, bool isVertical);
    /// Меню ячейки: строковые операции, экспорт/импорт, выборка, связанные формы.
    void prepareForShow(const MenuContext& ctx, QMenu* qtitanMenu);

    /// Меню заголовка колонки: описание, сумма, выравнивание, CF, прямой ввод.
    /// @param col  указатель на RCol (действителен на время показа меню)
    void prepareForHeader(ModelColumn column, const RCol* col, QMenu* menu);
signals:
    // Сигналы для операций со строками
    void sig_selection(ModelColumn col);
    // Сигналы для вспомогательных форм
    void sig_colProp(ModelColumn col);
    void sig_exportCsv();
    void sig_importCsv();
    // Настройка отображения
    void sig_widthByTemplate();
    void sig_widthByData();
    void sig_directCodeToggle(ModelColumn col);
    void sig_condFormatsEdit(ModelColumn col);

private:
    /// Удаляет из menu нежелательные встроенные пункты Qtitan.
    /// НЕ вызывает deleteLater — Qtitan владеет этими объектами.
    void removeUnwantedBuiltins(QMenu* menu);

    void buildStaticActions();
    void rebuildLinkedSubmenus(int contextRow);
    std::tuple<int, double> calcSumSelected() const;

    Qtitan::GridTableView* m_view;
    LinkedFormController*  m_linkedFormCtrl;

    // ── Динамические пункты (обновляются при каждом вызове) ────────────
    QAction* m_actDesc    = nullptr;   ///< описание колонки
    QAction* m_actSum     = nullptr;   ///< сумма выделенных
    QAction* m_actDirect  = nullptr;   ///< прямой ввод кода

    // ── Подменю связанных форм / макросов ───────────────────────────────
    QMenu*   m_linkedFormsMenu  = nullptr;
    QMenu*   m_linkedMacrosMenu = nullptr;

    // Статические экшны
    QAction* m_actTmpl      = nullptr;
    QAction* m_actData      = nullptr;
    QAction* m_actExport    = nullptr;
    QAction* m_actImport    = nullptr;
    QAction* m_actSel       = nullptr;
    QAction* m_actCF        = nullptr;
    // ── Хранители для отсоединения сигналов ────────────────────────────
    ModelColumn m_currentCol;   ///< col при последнем вызове prepareForShow

    RtabController::CommonTableActions m_comTabAct;
};
