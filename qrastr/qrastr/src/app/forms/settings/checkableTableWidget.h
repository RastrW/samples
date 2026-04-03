#pragma once
#include "settingsStackedItemWidget.h"

class QTableWidgetItem;
class QTableWidget;

/**
 * @brief Переиспользуемый виджет: таблица с двумя колонками —
 *        чекбокс и текстовое значение.
 * Жизненный цикл:
 *  1. Создать экземпляр.
 *  2. Вызвать populate(allItems, checkedItems) для заполнения.
 *  3. После подтверждения пользователем вызвать checkedItems()
 *     и сохранить результат в Params.
 */
class CheckableTableWidget : public SettingsStackedItemWidget {
    Q_OBJECT

public:
    explicit CheckableTableWidget(QWidget* parent = nullptr);

    /**
     * @brief Заполнить таблицу.
     * @param allItems      Полный список доступных элементов.
     * @param checkedItems  Подмножество, которое нужно отметить галочкой.
     */
    void populate(const std::vector<std::string>& allItems,
                  const std::vector<std::string>& checkedItems);

    /**
     * @brief Вернуть список отмеченных элементов.
     * @return Только те строки, у которых стоит галочка.
     */
    [[nodiscard]] std::vector<std::string> checkedItems() const;

    /**
     * @brief applyChanges() здесь ничего не делает — логика сохранения
     *        остаётся в классах-наследниках (SettingsOnLoad*Widget),
     *        которые знают, куда именно писать результат.
     */
    void applyChanges() override {}

private slots:
    void onItemChanged(QTableWidgetItem* item);

private:
    void setupUI();

    QTableWidget* m_table {nullptr};

    static constexpr int COL_CHECK = 0;
    static constexpr int COL_NAME  = 1;
};