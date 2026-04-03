#pragma once
#include "checkableTableWidget.h"

class QTableWidgetItem;
class QTableWidget;

/**
 * @class SettingsOnLoadTemplatesWidget
 * @brief Виджет выбора шаблонов для загрузки при старте.
 * Знает только о том, откуда брать данные и куда их сохранять
 */
class SettingsOnLoadTemplatesWidget : public CheckableTableWidget {
    Q_OBJECT
public:
    explicit SettingsOnLoadTemplatesWidget(QWidget* parent = nullptr);

    /// Сохраняет отмеченные шаблоны в Params
    void applyChanges() override;
};
