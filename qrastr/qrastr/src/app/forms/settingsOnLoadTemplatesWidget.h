#pragma once
#include "settingsStackedItemWidget.h"

#include "params.h"

class QTableWidgetItem;
class QTableWidget;

/// @class Виджет для выбора шаблонов, загружаемых при старте приложения
class SettingsOnLoadTemplatesWidget : public SettingsStackedItemWidget {
    Q_OBJECT

public:
    explicit SettingsOnLoadTemplatesWidget(QWidget *parent = nullptr);
    ~SettingsOnLoadTemplatesWidget() = default;

    /// Обновить содержимое при показе виджета
    void applyChanges() override;
signals:
    /// Сигнал об изменении выбранных шаблонов
    void settingsChanged();

private slots:
    /// Обработчик изменения состояния чекбокса в таблице
    void onItemChanged(QTableWidgetItem* item);

private:
    /// Создать и настроить UI элементы
    void setupUI();

    /// Заполнить таблицу доступными шаблонами
    void populateTemplates();

private:
    QTableWidget* pTableWidget_;  ///< Таблица с чекбоксами шаблонов
    Params::_v_templates
        m_pendingTemplates;  // Временное хранилище
    bool m_hasChanges {false};

    /// Индексы колонок в таблице
    static constexpr const int COLUMN_CHECKED = 0;
    static constexpr const int COLUMN_NAME = 1;
};

