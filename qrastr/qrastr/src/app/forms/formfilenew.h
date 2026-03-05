#pragma once

#include <QDialog>
#include <set>

class QTableWidget;

class FormFileNew : public QDialog
{
    Q_OBJECT
public:
    using _s_checked_templatenames = std::set<std::string>;
    explicit FormFileNew(QWidget *parent = nullptr);
    virtual ~FormFileNew() = default;

    _s_checked_templatenames getCheckedTemplateNames() const;

    static constexpr const int n_colnum_checked_      = 0;
    static constexpr const int n_colnum_templatename_ = 1;

private:
    QTableWidget* twTemplates;
};
