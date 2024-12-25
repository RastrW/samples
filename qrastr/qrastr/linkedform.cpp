#include "linkedform.h"
#include "rtabwidget.h"

//LinkedForm::LinkedForm() {}

void LinkedForm::FillBindVals()
{
    vbindvals.clear();
    for (const auto key : split( bind ,','))
    {
        int col = pbaseform->prm->getRdata()->mCols_.at(key);
        long val = std::visit(ToLong(),pbaseform->prm->getRdata()->pnparray_->Get(row,col));
        vbindvals.push_back(val);
    }
}
