#pragma once
#include <QObject>

class RCol;
class LinkedFormController;
class QMenu;

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

    /**
     * Заполняет menu, не очищая его.
     * Встроенные пункты Qtitan сохраняются; нежелательные удаляются.
     * Все создаваемые QAction имеют parent=menu → утечек нет.
     */
    void populate(QMenu* menu, const MenuContext& ctx);

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

    std::tuple<int, double> calcSumSelected() const;

    Qtitan::GridTableView*   m_view;
    LinkedFormController*    m_linkedFormCtrl;
    QWidget*                 m_parentWidget;
};
