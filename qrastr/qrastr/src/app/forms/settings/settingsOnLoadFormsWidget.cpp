#include "settingsOnLoadFormsWidget.h"
#include "rastrParameters.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <spdlog/spdlog.h>
#include "rastrParameters.h"

SettingsOnLoadFormsWidget::SettingsOnLoadFormsWidget(QWidget* parent)
    : CheckableTableWidget(parent)
{
    const auto& allForms     = RastrParameters::get_instance()->getFormsExists();
    const auto& checkedForms = RastrParameters::get_instance()->getStartLoadForms();

    populate(allForms, checkedForms);
}

void SettingsOnLoadFormsWidget::applyChanges() {
    RastrParameters::get_instance()->setStartLoadForms(checkedItems());
}