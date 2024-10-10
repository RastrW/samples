#ifndef FORMFILENEW_H
#define FORMFILENEW_H

#include <QDialog>
#include <set>

namespace Ui {
class FormFileNew;
}
//////////////////////////////////
// https://evileg.com/en/post/78/
///////////////////////////////////
class FormFileNew
    : public QDialog{
    Q_OBJECT
public:
    using _s_checked_templatenames = std::set<std::string>;
    explicit FormFileNew(QWidget *parent = nullptr);
    ~FormFileNew();
    _s_checked_templatenames getCheckedTemplateNames() const;
    static constexpr const int n_colnum_checked_      = 0;
    static constexpr const int n_colnum_templatename_ = 1;
private:
    Ui::FormFileNew* ui;
};

#endif // FORMFILENEW_H
