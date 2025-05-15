#ifndef PLUGIN_TI_INTERFACES_H
#define PLUGIN_TI_INTERFACES_H
#pragma once

//QT_BEGIN_NAMESPACE
#include <QtPlugin>
//QT_END_NAMESPACE

class IPlainTI;
namespace spdlog{ class logger;}
class InterfaceTI{
    public:
    virtual ~InterfaceTI() = default;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger)=0;
    virtual std::shared_ptr<IPlainTI> getIPlainTIPtr()=0;
};

QT_BEGIN_NAMESPACE
#define InterfaceTI_iid "TI.Plugins.InterfaceTI/1.0"
Q_DECLARE_INTERFACE( InterfaceTI, InterfaceTI_iid )
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACES_H
