#pragma once
#include "QtitanGrid.h"

class SearchableComboPopupTwo;

// SearchableComboEditorTwo — редактор ячейки
class SearchableComboEditorTwo : public Qtitan::GridEditorBase
{
public:
    void createEditModeContext()  override;
    void destroyEditModeContext() override;
    void setValueToWidget(const QVariant& value) override;
    QVariant getContextValue()  const override;
    bool     isContextModified()      override;

private:
    void showPopup();

    SearchableComboPopupTwo* m_popup        = nullptr;
    int                      m_currentKey   = -1;   ///< ключ из модели до редактирования
    int                      m_selectedKey  = -1;   ///< -1 = пользователь ничего не выбрал
    bool                     m_clearChosen  = false; ///< true = выбрано "Не задано"
};