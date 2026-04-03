#pragma once
#include "checkableTableWidget.h"

class QTableWidgetItem;
class QTableWidget;

/**
 * @brief Виджет выбора форм для загрузки при старте.
 * Знает только о том, откуда брать данные и куда их сохранять
*/
class SettingsOnLoadFormsWidget : public CheckableTableWidget {
    Q_OBJECT
public:
    explicit SettingsOnLoadFormsWidget(QWidget* parent = nullptr);

    /// Сохраняет отмеченные формы в Params
    void applyChanges() override;
};

