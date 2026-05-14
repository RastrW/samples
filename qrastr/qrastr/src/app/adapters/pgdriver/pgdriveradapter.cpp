#include "pgdriveradapter.h"
#include "qpgdriver.h"

PGDriverAdapter::PGDriverAdapter(std::shared_ptr<QPGDriver> qpgdriver)
    : m_qpgdriver(std::move(qpgdriver))
{
    assert(m_qpgdriver != nullptr);
}
int PGDriverAdapter::Init(){
    return m_qpgdriver->Init();
}
int PGDriverAdapter::Connect(){
    return m_qpgdriver->Connect();
}
int PGDriverAdapter::Table_R2SQL(std::string Table, std::string cols, std::string Viborka){
    return m_qpgdriver->Table_R2SQL(Table,cols,Viborka) ;
}
int PGDriverAdapter::Table_SQL2R(std::string Table, std::string cols, std::string Viborka){
    return m_qpgdriver->Table_SQL2R(Table,cols,Viborka) ;
}
int PGDriverAdapter::All_R2SQL(std::string TemplateName){
    return m_qpgdriver->All_R2SQL(TemplateName);
}
int PGDriverAdapter::All_SQL2R(std::string TemplateName){
    return m_qpgdriver->All_SQL2R(TemplateName);
}
