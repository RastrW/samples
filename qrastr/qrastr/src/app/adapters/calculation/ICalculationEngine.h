#pragma once
#include <astra/IPlainRastr.h>

class ICalculationEngine {
public:
    virtual ~ICalculationEngine() = default;

    virtual eASTCode Rgm (std::string_view params = {}) = 0;
    virtual eASTCode Kdd (std::string_view params = {}) = 0;
    virtual eASTCode Opf (std::string_view params = {}) = 0;
    virtual eASTCode SMZU(std::string_view params = {}) = 0;

    virtual eASTCode Kz(std::string_view params,
                        eNonsym nonsym,
                        long p1, long p2, long p3,
                        double lengthFromP1InProc,
                        double rd,
                        double z_re, double z_im) = 0;

    virtual void CalcIdop(double           temp,
                          double           pcab,
                          std::string_view selection     = {},
                          bool             ignoreTTables = false) = 0;
};