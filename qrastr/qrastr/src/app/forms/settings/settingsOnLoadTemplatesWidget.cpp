#include "settingsOnLoadTemplatesWidget.h"

#include <spdlog/spdlog.h>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include "rastrParameters.h"

SettingsOnLoadTemplatesWidget::SettingsOnLoadTemplatesWidget(QWidget* parent)
    : CheckableTableWidget(parent)
{
    // Собираем полный список из расширений шаблонов
    std::vector<std::string> allItems;
    for (const auto& [name, ext] : RastrParameters::get_instance()->getTemplateExts())
        allItems.emplace_back(name + ext);

    // Отмеченные — из настроек
    const auto& checked = RastrParameters::get_instance()->getStartLoadTemplates();

    populate(allItems, checked);
}

void SettingsOnLoadTemplatesWidget::applyChanges() {
    RastrParameters::get_instance()->setStartLoadTemplates(checkedItems());
}
