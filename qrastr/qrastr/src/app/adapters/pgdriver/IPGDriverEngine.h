#pragma once
#include <string_view>
#include <string>

class IPGDriverEngine {
public:
    virtual ~IPGDriverEngine() = default;

    virtual int Init() = 0;
    virtual int Connect() = 0;
    virtual int Table_R2SQL(std::string Table, std::string cols, std::string Viborka) = 0;
    virtual int Table_SQL2R(std::string Table, std::string cols, std::string Viborka) = 0;
    virtual int All_R2SQL(std::string TemplateName)                                   = 0;
    virtual int All_SQL2R(std::string TemplateName)                                   = 0;
};
