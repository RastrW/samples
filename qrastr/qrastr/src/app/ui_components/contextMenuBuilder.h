#pragma once
#include <QObject>

class RCol;
class LinkedFormController;
class QMenu;
class QAction;

namespace Qtitan{ class GridTableView; }

/// Контекст одного показа меню.
struct MenuContext {
    int   column = -1;
    int   row    = -1;
    RCol* col    = nullptr;   ///< указатель действителен на время показа меню
};

class ContextMenuBuilder : public QObject
{
    Q_OBJECT
public:
    explicit ContextMenuBuilder(Qtitan::GridTableView*   view,
                                LinkedFormController*    linkedFormCtrl,
                                QObject*                 parent = nullptr);

    /// Строит персистентный QMenu и все статичные QAction.
    void initMenu(QWidget* menuParent);
    /// обновляет динамические пункты.
    QMenu* prepareForShow(const MenuContext& ctx);
signals:
    // Сигналы для операций со строками
    void sig_addRow();
    void sig_insertRow();
    void sig_deleteRow();
    void sig_duplicateRow();
    void sig_groupCorrection();
    // Сигналы для вспомогательных форм
    void sig_colProp(int col);
    void sig_selection();
    void sig_exportCsv();
    void sig_importCsv();
    // Настройка отображения
    void sig_widthByTemplate();
    void sig_widthByData();
    void sig_directCodeToggle(int col);
    void sig_condFormatsEdit(int col);

private:
    /// Удаляет из menu нежелательные встроенные пункты Qtitan.
    /// НЕ вызывает deleteLater — Qtitan владеет этими объектами.
    void removeUnwantedBuiltins(QMenu* menu);

    void buildStaticActions();
    void rebuildLinkedSubmenus(int contextRow);
    std::tuple<int, double> calcSumSelected() const;

    Qtitan::GridTableView* m_view;
    LinkedFormController*  m_linkedFormCtrl;

    // ── Персистентное меню ──────────────────────────────────────────────
    QMenu* m_menu         = nullptr;   ///< живёт столько же, сколько builder

    // ── Динамические пункты (обновляются при каждом вызове) ────────────
    QAction* m_actDesc    = nullptr;   ///< описание колонки
    QAction* m_actSum     = nullptr;   ///< сумма выделенных
    QAction* m_actDirect  = nullptr;   ///< прямой ввод кода

    // ── Подменю связанных форм / макросов ───────────────────────────────
    QMenu*   m_linkedFormsMenu  = nullptr;
    QMenu*   m_linkedMacrosMenu = nullptr;

    // ── Хранители для отсоединения сигналов ────────────────────────────
    int      m_currentCol = -1;   ///< col при последнем вызове prepareForShow
};
