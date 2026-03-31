#include "settingsOnLoadTemplatesWidget.h"

#include <spdlog/spdlog.h>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include "params.h"

SettingsOnLoadTemplatesWidget::SettingsOnLoadTemplatesWidget(QWidget* parent)
    : CheckableTableWidget(parent)
{
    // Собираем полный список из расширений шаблонов
    std::vector<std::string> allItems;
    for (const auto& [name, ext] : Params::get_instance()->getTemplateExts())
        allItems.emplace_back(name + ext);

    // Отмеченные — из настроек
    const auto& checked = Params::get_instance()->getStartLoadTemplates();

    populate(allItems, checked);
}

void SettingsOnLoadTemplatesWidget::applyChanges() {
    Params::get_instance()->setStartLoadTemplates(checkedItems());
}
