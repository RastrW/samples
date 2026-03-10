#ifndef PLUGINRASTR_H
#define PLUGINRASTR_H
#pragma once

#include <QObject>
#include <QtPlugin>
#include "plugin_interfaces.h"
#include <mutex>
#include <condition_variable>

class PluginRastr 
	: public QObject
	, InterfaceRastr {
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "Rastr.Plugins.InterfaceRastr" FILE "pluginrastr.json" )
    Q_INTERFACES(InterfaceRastr)
public:
    typedef IPlainRastr* (*_prf)() ;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger) override;
    virtual std::shared_ptr<IPlainRastr> getIPlainRastrPtr() override;
    //graph
   // void AsyncCallback2(int iMSG, const char *params);
    std::mutex ws_mutex;
    std::condition_variable ws_cv;
    std::map<int, std::string> calls;
};

/*
class IPlainRastr;
class qAstra : public QObject{
    Q_OBJECT
public:
    typedef IPlainRastr* (__cdecl* _prf)() ;
    explicit qAstra(QObject *parent = nullptr);
    virtual ~qAstra() = default;
    int tst_iplainrastr() const;
    std::shared_ptr<IPlainRastr> getPlainRastrSharedPtr();
signals:

};//class qAstra
*/

#endif
