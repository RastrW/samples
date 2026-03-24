#pragma once
#include "settingsStackedItemWidget.h"

class QTableWidgetItem;
class QTableWidget;

/// @class Виджет для выбора форм, загружаемых при старте приложения
class SettingsOnLoadFormsWidget : public SettingsStackedItemWidget {
    Q_OBJECT

public:
    explicit SettingsOnLoadFormsWidget(QWidget *parent = nullptr);
    ~SettingsOnLoadFormsWidget() = default;

    /// Обновить содержимое при показе виджета
    void applyChanges() override;
private slots:
    /// Обработчик изменения состояния чекбокса в таблице
    void onItemChanged(QTableWidgetItem* item);

private:
    /// Создать и настроить UI элементы
    void setupUI();

    /// Заполнить таблицу доступными формами
    void populateForms();

private:
    QTableWidget* pTableWidget_;  ///< Таблица с чекбоксами форм
    std::vector<std::string>
                m_pendingForms;  // Временное хранилище
    bool m_hasChanges {false};
    /// Индексы колонок в таблице
    static constexpr const int COLUMN_CHECKED = 0;      ///< Колонка с чекбоксом
    static constexpr const int COLUMN_NAME = 1;         ///< Колонка с именем формы
};

