#include "formsettingsstackeditem.h"
#include "formsettings.h"

FormSettingsStackedItem::FormSettingsStackedItem(FormSettings* pfm_settings)
    : pfm_settings_{pfm_settings}{
}
FormSettings* FormSettingsStackedItem::getFormSettings(){
    return pfm_settings_;
}
