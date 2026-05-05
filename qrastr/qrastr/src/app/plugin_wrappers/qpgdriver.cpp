#include "qpgdriver.h"

QPGDriver::QPGDriver(QObject *parent)
    : QObject{parent}
{
}

void QPGDriver::setPGDriver(const _sp_pgdriver& sp_pgdriver_in)
{
    sp_pgdriver_ = sp_pgdriver_in;
}
QPGDriver::_sp_pgdriver QPGDriver::getPGDriver() const
{
    return sp_pgdriver_;
}
int QPGDriver::Init()
{
    return sp_pgdriver_->Init();
}
int QPGDriver::Connect()
{
    return sp_pgdriver_->Connect();
}
int QPGDriver::Table_R2SQL(std::string Table, std::string cols, std::string Viborka)
{
    return sp_pgdriver_->Table_R2SQL(Table,cols,Viborka);
}
int QPGDriver::Table_SQL2R(std::string Table, std::string cols, std::string Viborka)
{
    return sp_pgdriver_->Table_SQL2R(Table,cols,Viborka);
}
int QPGDriver::All_R2SQL(std::string TemplateName)
{
    return sp_pgdriver_->All_R2SQL(TemplateName);
}
int QPGDriver::All_SQL2R(std::string TemplateName)
{
    return sp_pgdriver_->All_SQL2R(TemplateName);
}
