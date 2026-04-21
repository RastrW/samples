#pragma once

#include <QWidget>
#include <QToolBar>
#include <QLabel>

class RGrid;
class RModel;
class FilterManager;
class RtabController;

namespace Qtitan { class GridTableView; }

/**
 * @brief Визуальная оболочка таблицы Rastr — то, что видит пользователь в Dock.
 * Ответственности RtabShell:
 *   - построить layout (toolbar → autofilter → grid → statusbar);
 *   - построить QToolBar и подключить его action-ы к слотам RtabController;
 *   - зарегистрировать горячие клавиши (только когда withToolbar = true);
 *   - реализовать функцию копирования по кнопке из QToolBar
 *   - обновлять statusbar при изменении числа строк.
 */
class RtabShell : public QWidget
{
    Q_OBJECT
public:
    /**
     * @param controller    — владелец; к его слотам подключаются action-ы
     * @param withToolbar   — false для связанных форм (без тулбара и шорткатов)
     */
    explicit RtabShell(RGrid*                 grid,
                       Qtitan::GridTableView* view,
                       RModel*                model,
                       FilterManager*         filterManager,
                       RtabController*        controller,
                       bool                   withToolbar,
                       QWidget*               parent = nullptr);

public slots:
    void slot_updateStatusLabel();
    void slot_copyToClipboard();
    void slot_showSearchMenu();

private:
    void buildToolbar(RtabController* controller);
    void buildLayout(bool withToolbar);

    RGrid*                 m_grid;
    Qtitan::GridTableView* m_view;
    RModel*                m_model;
    FilterManager*         m_filterManager;

    QToolBar* m_toolbar     {nullptr};
    QLabel*   m_statusLabel {nullptr};
};
