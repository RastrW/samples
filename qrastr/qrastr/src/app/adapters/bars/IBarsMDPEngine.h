#pragma once
#include <string_view>

class IBarsMDPEngine {
public:
    virtual ~IBarsMDPEngine() = default;

    virtual void Init            (std::string_view sections) = 0;
    virtual void UpdateMDPFields ()                          = 0;
    virtual void UpdateAUTOFields()                          = 0;
};