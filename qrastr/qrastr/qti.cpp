#include "qti.h"

QTI::QTI(QObject *parent)
    : QObject{parent}
{
}

void QTI::setTI(const _sp_ti& sp_ti_in)
{
      sp_ti_ = sp_ti_in;
}
QTI::_sp_ti QTI::getTI() const
{
    return sp_ti_;
}
long QTI::RecalcDor() const
{
    return sp_ti_->RecalcDor();
}
long QTI::UpdateTables() const
{
    return sp_ti_->UpdateTables();
}
long QTI::CalcPTI() const
{
    return sp_ti_->CalcPTI();
}
long QTI::DobavPTI() const
{
    return sp_ti_->DobavPTI();
}
long QTI::FiltrTI() const
{
    return sp_ti_->FiltrTI();
}
