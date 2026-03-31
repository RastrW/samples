#include "settingsOnLoadFormsWidget.h"
#include "params.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <spdlog/spdlog.h>
#include "params.h"

SettingsOnLoadFormsWidget::SettingsOnLoadFormsWidget(QWidget* parent)
    : CheckableTableWidget(parent)
{
    const auto& allForms     = Params::get_instance()->getFormsExists();
    const auto& checkedForms = Params::get_instance()->getStartLoadForms();

    populate(allForms, checkedForms);
}

void SettingsOnLoadFormsWidget::applyChanges() {
    Params::get_instance()->setStartLoadForms(checkedItems());
}