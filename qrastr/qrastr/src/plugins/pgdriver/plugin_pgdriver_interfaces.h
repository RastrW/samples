#pragma once

#include <QtPlugin>

class IPlainPGDriver;
namespace spdlog{ class logger;}
class InterfacePGDriver{
public:
    virtual ~InterfacePGDriver() = default;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger)=0;
    virtual std::shared_ptr<IPlainPGDriver> getIPlainPGDriverPtr()=0;
};

QT_BEGIN_NAMESPACE
#define InterfacePGDriver_iid "PGDriver.Plugins.InterfacePGDriver/1.0"
Q_DECLARE_INTERFACE( InterfacePGDriver, InterfacePGDriver_iid )
QT_END_NAMESPACE
