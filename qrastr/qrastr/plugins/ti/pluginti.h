#ifndef PLUGINTI_H
#define PLUGINTI_H
#pragma once

#include <QObject>
#include <QtPlugin>
#include "plugin_ti_interfaces.h"

class PluginTI
    : public QObject
    , InterfaceTI {
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "TI.Plugins.InterfaceTI" FILE "pluginti.json" )
    Q_INTERFACES(InterfaceTI)
public:
    typedef IPlainTI* (*_ptif)() ;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger) override;
    virtual std::shared_ptr<IPlainTI> getIPlainTIPtr() override;
};

#endif // PLUGINTI_H
