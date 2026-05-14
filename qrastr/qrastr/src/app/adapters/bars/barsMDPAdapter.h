#pragma once
#include "IBarsMDPEngine.h"
#include <memory>

class QBarsMDP;

class BarsMDPAdapter : public IBarsMDPEngine {
public:
    explicit BarsMDPAdapter(std::shared_ptr<QBarsMDP> qbarsmdp);

    void Init            (std::string_view sections) override;
    void UpdateMDPFields ()                          override;
    void UpdateAUTOFields()                          override;

private:
    std::shared_ptr<QBarsMDP> m_qbarsmdp;
};