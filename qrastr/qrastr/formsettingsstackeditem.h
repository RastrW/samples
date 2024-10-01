#ifndef FORMSETTINGSSTACKEDITEM_H
#define FORMSETTINGSSTACKEDITEM_H


class FormSettings;
class FormSettingsStackedItem
{
public:
    FormSettingsStackedItem(FormSettings* pfm_settings);
    FormSettings* getFormSettings();
private:
    FormSettings* pfm_settings_ = nullptr;
};

#endif // FORMSETTINGSSTACKEDITEM_H
