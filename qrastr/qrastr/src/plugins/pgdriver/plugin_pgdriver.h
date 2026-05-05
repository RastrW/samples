#pragma once

#include <QObject>
#include <QtPlugin>
#include "plugin_pgdriver_interfaces.h"

class plugin_pgdriver :
      public QObject
    , InterfacePGDriver {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID InterfacePGDriver_iid FILE "pgdriver.json")
    Q_INTERFACES(InterfacePGDriver)
public:
    typedef IPlainPGDriver* (*_ppgdriverf)() ;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger) override;
    virtual std::shared_ptr<IPlainPGDriver> getIPlainPGDriverPtr() override;
};
