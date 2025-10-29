#ifndef PLUGIN_BARSMDP_H
#define PLUGIN_BARSMDP_H
#pragma once

#include <QObject>
#include <QtPlugin>
#include "plugin_barsmdp_interfaces.h"

class PluginBarsMDP
    : public QObject
    , InterfaceBarsMDP {
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "BarsMDP.Plugins.InterfaceBarsMDP" FILE "pluginbarsmdp.json" )
    Q_INTERFACES(InterfaceBarsMDP)
public:
    typedef IPlainBarsMDP* (*_pbarsmdpf)() ;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger) override;
    virtual std::shared_ptr<IPlainBarsMDP> getIPlainBarsMDPPtr() override;
};

#endif // PLUGIN_BARSMDP_H

