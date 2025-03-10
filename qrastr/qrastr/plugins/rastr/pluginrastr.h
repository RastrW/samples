#ifndef PLUGINRASTR_H
#define PLUGINRASTR_H

#include <QObject>
#include <QtPlugin>

#include "plugin_interfaces.h"

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
