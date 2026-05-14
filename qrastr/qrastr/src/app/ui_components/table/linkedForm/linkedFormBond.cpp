#include "linkedFormBond.h"
#include <DockWidget.h>
#include <QtitanGrid.h>
#include "table/rtabcontroller.h"

LinkedFormBond::LinkedFormBond(RtabController*        parentCtrl,
                               Qtitan::GridTableView* parentView,
                               RtabController*        childCtrl,
                               ads::CDockWidget*      childDock)
    : QObject(parentCtrl)
    , m_childCtrl(childCtrl)
    , m_childDock(childDock)
{
    // 1. Смена строки в родителе → обновление фильтра дочерней формы
    m_focusConn = connect(
        parentView, &Qtitan::GridTableView::focusRowChanged,
        childCtrl,  [parentView, childCtrl](int, int) {
            GridRow row = parentView->focusedRow();
            if (!row.isValid()) return;
            const int modelRow = row.modelIndex().row();
            if (modelRow >= 0)
                childCtrl->notifyParentRowChanged(modelRow);
        });

    // 2. Дочерний dock закрылся → bond больше не нужен, удаляем себя
    connect(childDock, &ads::CDockWidget::closed,
            this, &QObject::deleteLater);

    // 3. Родительский контроллер уничтожается → снимаем фильтр с дочернего.
    //    - destroyed гарантированно вызывается из деструктора QObject
    //    - не зависит от порядка закрытия dock-ов
    //    - bond сам является child parentCtrl, поэтому destroyed
    //      придёт до уничтожения bond-а (Qt сначала испускает destroyed,
    //      потом удаляет детей)
    connect(parentCtrl, &QObject::destroyed,
            this, [this]() {
                removeChildFilter();
                // Соединение с focusRowChanged уже мертво (parentView удалён),
                // явно отключаем чтобы не было dangling
                QObject::disconnect(m_focusConn);
            });
}

void LinkedFormBond::removeChildFilter()
{
    // childCtrl мог уже умереть если дочерний dock закрыли раньше родителя.
    // Проверяем через childDock: если он ещё жив — снимаем фильтр.
    if (!m_childDock || !m_childCtrl) return;

    m_childCtrl->clearLinkedFilter();
}
