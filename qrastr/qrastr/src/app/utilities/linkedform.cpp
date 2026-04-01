#include "linkedform.h"
#include "rtabwidget.h"

void LinkedForm::FillBindVals()
{
    vbindvals.clear();
    for (const auto& key : split( bind ,','))
    {
        long val = pbaseform->getLongValue(key, row);
        vbindvals.push_back(val);
    }
}
