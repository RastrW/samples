#pragma once
#include "checkboxListDialog.h"
#include <set>

class QTableWidget;
class QTableWidgetItem;
class QCheckBox;


class FileNewDialog : public CheckboxListDialog
{
    Q_OBJECT
public:
    using _s_checked_templatenames = std::set<std::string>;

    explicit FileNewDialog(QWidget *parent = nullptr);
    virtual ~FileNewDialog() = default;

    _s_checked_templatenames getCheckedTemplateNames() const;

    static constexpr const int n_colnum_checked_      = 0;
    static constexpr const int n_colnum_templatename_ = 1;
};
