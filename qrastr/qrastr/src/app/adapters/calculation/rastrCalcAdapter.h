#pragma once
#include "ICalculationEngine.h"
#include <memory>

class QAstra;

class RastrCalcAdapter : public ICalculationEngine {
public:
    explicit RastrCalcAdapter(std::shared_ptr<QAstra> qastra);

    eASTCode Rgm (std::string_view params = {}) override;
    eASTCode Kdd (std::string_view params = {}) override;
    eASTCode Opf (std::string_view params = {}) override;
    eASTCode SMZU(std::string_view params = {}) override;
    eASTCode Kz  (std::string_view params, eNonsym nonsym,
                  long p1, long p2, long p3,
                  double lengthFromP1InProc, double rd,
                  double z_re, double z_im) override;
    void CalcIdop(double temp, double pcab,
                  std::string_view selection     = {},
                  bool             ignoreTTables = false) override;

private:
    std::shared_ptr<QAstra> m_qastra;
};