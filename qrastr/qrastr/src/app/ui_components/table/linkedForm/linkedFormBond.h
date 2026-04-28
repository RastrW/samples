#pragma once
#include <QObject>
namespace Qtitan   { class GridTableView; }
namespace ads      {class CDockWidget;}
class RtabController;

/**
 * @brief Связь между родительской и дочерней таблицей.
 *
 * Живёт как QObject с parent = parentCtrl (удалится вместе с ним).
 * При уничтожении родителя: снимает фильтр с дочернего.
 * При уничтожении дочернего: bond удаляет себя сам (deleteLater).
 *
 * Не хранится ни в родителе, ни в дочернем — управляет собой сам.
 */
class LinkedFormBond : public QObject
{
    Q_OBJECT
public:
    /**
     * @param parentCtrl   родительский контроллер (становится QObject parent-ом)
     * @param parentView   view родителя — источник focusRowChanged
     * @param childCtrl    дочерний контроллер
     * @param childDock    dock дочерней формы (нужен для снятия фильтра при закрытии)
     */
    explicit LinkedFormBond(RtabController*        parentCtrl,
                            Qtitan::GridTableView* parentView,
                            RtabController*        childCtrl,
                            ads::CDockWidget*      childDock);
private:
    void removeChildFilter();

    RtabController*        m_childCtrl {nullptr};  // не владеет
    ads::CDockWidget*      m_childDock {nullptr};  // не владеет
    QMetaObject::Connection m_focusConn;           // focusRowChanged → child
};