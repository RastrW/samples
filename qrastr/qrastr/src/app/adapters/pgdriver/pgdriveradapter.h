#pragma once
#include "IPGDriverEngine.h"
#include <memory>
class QPGDriver;

class PGDriverAdapter : public IPGDriverEngine {
public:
    explicit PGDriverAdapter(std::shared_ptr<QPGDriver> qpgdriver);

    int Init()                                                                override;
    int Connect()                                                             override;
    int Table_R2SQL(std::string Table, std::string cols, std::string Viborka) override;
    int Table_SQL2R(std::string Table, std::string cols, std::string Viborka) override;
    int All_R2SQL(std::string TemplateName)                                   override;
    int All_SQL2R(std::string TemplateName)                                   override;

private:
    std::shared_ptr<QPGDriver> m_qpgdriver;
};
