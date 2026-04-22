#pragma once

#include <QObject>
#include <QMenu>
#include <memory>
#include <string>

#include "linkedform.h"
#include "UIForms.h"

class QAstra;
class RTablesDataManager;
class RModel;
class RtabWidget;
class PyHlp;
class QWidget;

namespace ads      { class CDockManager;  }
namespace Qtitan   { class GridTableView; }

/**
 * @class Управляет функционалом связанных форм для одного RtabWidget.
 * Хранит текущую LinkedForm, строит подменю контекстного меню,
 * открывает дочерние формы в Dock и синхронизирует их фильтр
 * при смене строки в родительской форме.
 */
class LinkedFormController : public QObject
{
    Q_OBJECT

public:
    explicit LinkedFormController(
        QAstra*                  qastra,
        RTablesDataManager*      rtdm,
        ads::CDockManager*       dockManager,
        Qtitan::GridTableView*   view,
        RModel*                  model,
        const CUIForm&           form,
        RtabController*          parentController);

    ~LinkedFormController() override = default;
	
    //  Инициализация
    void setPyHlp(std::shared_ptr<PyHlp> pyHlp);

    //  Построение подменю контекстного меню
    /**
     * @brief Возвращает QMenu «Связанные формы».
     * @param contextRow  строка, на которой было открыто контекстное меню
     */
    void buildLinkedFormsMenu(int contextRow, QMenu* menu);

    /**
     * @brief Возвращает QMenu «Макрос».
     * @param contextRow  строка, на которой было открыто контекстное меню
     */
    void buildLinkedMacroMenu(int contextRow, QMenu* menu);
    //  Управление связанной формой
    /// @brief Применяет фильтр связанной формы к текущему представлению.
    void applyLinkedForm(LinkedForm lf);
    /// @brief Открывает дочернюю форму в нижней панели Dock.
    /// ТИ:Каналы ; id1=%d & id2=0 & prv_num<8 ; 801
    void openLinkedForm(LinkedForm lf);
    /// @brief Запускает Python-макрос из контекстного меню.
    void openLinkedMacro(LinkedMacro lm, int contextRow);
    //  Реакция на события родительской формы
    /**
     * @brief Вызывается при смене фокусной строки в РОДИТЕЛЬСКОЙ форме.
     * Пересчитывает bind-значения (FillBindVals) и повторно применяет
     * фильтр дочерней формы.
     * @param newRow  новый индекс строки
     */
    void onParentRowChanged(int newRow);
    /// @brief Отключает все Qt-соединения, хранящиеся в m_lf.vconn.
    void disconnectAll();
private:
    /// @brief Читает long-значение ячейки из кешированного DataBlock модели.
    int getLongValue(const std::string& col, long row);

    QAstra*                  m_qastra;
    RTablesDataManager*      m_rtdm;
    ads::CDockManager*       m_dockManager;
    Qtitan::GridTableView*   m_view;         ///< не владеет; живёт дольше
    RModel*                  m_model;        ///< не владеет; живёт дольше
    CUIForm                  m_form;
    QWidget*                 m_parentWidget;
    std::shared_ptr<PyHlp>   m_pyHlp;
    RtabController* m_parentController;

    LinkedForm               m_lf;           ///< текущая активная связанная форма
};
