#include "rastrCalcAdapter.h"
#include "QAstra.h"

RastrCalcAdapter::RastrCalcAdapter(std::shared_ptr<QAstra> qastra)
    : m_qastra(std::move(qastra))
{
    assert(m_qastra != nullptr);
}

eASTCode RastrCalcAdapter::Rgm(std::string_view params) {
    return m_qastra->Rgm(params);
}
eASTCode RastrCalcAdapter::Kdd(std::string_view params) {
    return m_qastra->Kdd(params);
}
eASTCode RastrCalcAdapter::Opf(std::string_view params) {
    return m_qastra->Opf(params);
}
eASTCode RastrCalcAdapter::SMZU(std::string_view params) {
    return m_qastra->SMZU(params);
}
eASTCode RastrCalcAdapter::Kz(std::string_view params, eNonsym nonsym,
                               long p1, long p2, long p3,
                               double lengthFromP1InProc, double rd,
                               double z_re, double z_im) {
    return m_qastra->Kz(params, nonsym, p1, p2, p3,
                        lengthFromP1InProc, rd, z_re, z_im);
}
void RastrCalcAdapter::CalcIdop(double temp, double pcab,
                                 std::string_view selection,
                                 bool ignoreTTables) {
    m_qastra->CalcIdop(temp, pcab, selection, ignoreTTables);
}