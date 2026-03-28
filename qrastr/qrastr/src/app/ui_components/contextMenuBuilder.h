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
    /// @brief обновляет динамические пункты.
    void prepareForShow(const MenuContext& ctx, QMenu* qtitanMenu);
    /// @brief Меню заголовка колонки.
    void prepareForHeader(int column, QMenu* menu);
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

    // ── Динамические пункты (обновляются при каждом вызове) ────────────
    QAction* m_actDesc    = nullptr;   ///< описание колонки
    QAction* m_actSum     = nullptr;   ///< сумма выделенных
    QAction* m_actDirect  = nullptr;   ///< прямой ввод кода

    // ── Подменю связанных форм / макросов ───────────────────────────────
    QMenu*   m_linkedFormsMenu  = nullptr;
    QMenu*   m_linkedMacrosMenu = nullptr;

    // Статические экшны
    QAction* m_actInsert    = nullptr;
    QAction* m_actAdd       = nullptr;
    QAction* m_actDuplicate = nullptr;
    QAction* m_actDelete    = nullptr;
    QAction* m_actGroup     = nullptr;
    QAction* m_actTmpl      = nullptr;
    QAction* m_actData      = nullptr;
    QAction* m_actExport    = nullptr;
    QAction* m_actImport    = nullptr;
    QAction* m_actSel       = nullptr;
    QAction* m_actCF        = nullptr;
    // ── Хранители для отсоединения сигналов ────────────────────────────
    int      m_currentCol = -1;   ///< col при последнем вызове prepareForShow
};
