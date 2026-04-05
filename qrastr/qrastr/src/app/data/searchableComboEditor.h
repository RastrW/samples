#pragma once
#include "QtitanGrid.h"

class SearchableComboPopup;

/// Сам редактор: открывает SearchableComboPopup поверх ячейки
class SearchableComboEditor : public Qtitan::GridEditorBase
{
public:
    void createEditModeContext()  override;
    void destroyEditModeContext() override;
    void setValueToWidget(const QVariant& value) override;

    // QTitan читает это при postEditor()
    QVariant getContextValue() const override;
    bool isContextModified() override;
private:
    void showPopup();

    SearchableComboPopup* m_popup       = nullptr;
    QString               m_current;
    // -1 = пользователь ничего не выбрал
    int                   m_selectedIdx = -1;
};