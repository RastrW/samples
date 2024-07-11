#include "filterlineedit.h"



FilterLineEdit::FilterLineEdit(QWidget* parent, std::vector<FilterLineEdit*>* filters, size_t columnnum) :
    QLineEdit(parent),
    filterList(filters),
    columnNumber(columnnum)
{
    setPlaceholderText(tr("Filter"));
    setProperty("column", static_cast<int>(columnnum));
}
