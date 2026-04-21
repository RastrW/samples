#pragma once
#include "QtitanGrid.h"

class SearchableComboPopupTwo;
class SearchableComboCell;

// SearchableComboEditorTwo — редактор ячейки
class SearchableComboEditorTwo : public Qtitan::GridEditorBase
{
public:
    SearchableComboEditorTwo() = default;

    // GridEditorBase interface
    void     createEditModeContext()  override;
    void     destroyEditModeContext() override;
    void     setValueToWidget(const QVariant& value) override;
    QVariant getContextValue()  const override;
    bool     isContextModified()      override;
    ///@note возвращает виджет, который Qtitan встраивает в ячейку/фильтр
    ///в ячейке обязательно должен быть фильтр, иначе краш при вызове фильтра
    QWidget* getCellWidget()          override { return m_container; }

private:
    //Открыть popup (можно открыть и редактировать множество раз, пока
    //фокус находится на выбранной ячейке)
    void showPopup();
    void updateDisplayText();

    QWidget*     m_container   = nullptr;
    QLineEdit*   m_lineEdit    = nullptr;
    QToolButton* m_button      = nullptr;

    SearchableComboPopupTwo* m_popup       = nullptr;
    int                      m_currentKey  = -1; ///< ключ из модели до редактирования
    int                      m_selectedKey = -1; ///< -1 = пользователь ничего не выбрал
    bool                     m_clearChosen = false; ///< true = выбрано "Не задано"
};