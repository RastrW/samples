#pragma once

#include <QtPlugin>

class IPlainBarsMDP;
namespace spdlog{ class logger;}
class InterfaceBarsMDP{
public:
    virtual ~InterfaceBarsMDP() = default;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger)=0;
    virtual std::shared_ptr<IPlainBarsMDP> getIPlainBarsMDPPtr()=0;
};

QT_BEGIN_NAMESPACE
#define InterfaceBarsMDP_iid "BarsMDP.Plugins.InterfaceBarsMDP/1.0"
Q_DECLARE_INTERFACE( InterfaceBarsMDP, InterfaceBarsMDP_iid )
QT_END_NAMESPACE
