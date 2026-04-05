#include "qbarsmdp.h"

QBarsMDP::QBarsMDP(QObject *parent)
    : QObject{parent}
{
}

void QBarsMDP::setBarsMDP(const _sp_barsmdp& sp_barsmdp_in)
{
    sp_barsmdp_ = sp_barsmdp_in;
}
QBarsMDP::_sp_barsmdp QBarsMDP::getBarsMDP() const
{
    return sp_barsmdp_;
}
long QBarsMDP::Init(std::string _Sech)
{
    return sp_barsmdp_->Init(_Sech);
}
long QBarsMDP::UpdateMDPFields()
{
    return sp_barsmdp_->UpdateMDPFields();
}
long QBarsMDP::UpdateAUTOFields()
{
    return sp_barsmdp_->UpdateAUTOFields();
}
