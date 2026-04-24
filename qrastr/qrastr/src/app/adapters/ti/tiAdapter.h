#pragma once
#include "ITIEngine.h"
#include <memory>

class QTI;

class TIAdapter : public ITIEngine {
public:
    explicit TIAdapter(std::shared_ptr<QTI> qti);

    long RecalcDor()    override;
    long UpdateTables() override;
    long CalcPTI()      override;
    long DobavPTI()     override;
    long FiltrTI()      override;

private:
    std::shared_ptr<QTI> m_qti;
};