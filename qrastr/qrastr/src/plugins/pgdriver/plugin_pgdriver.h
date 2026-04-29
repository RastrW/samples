#ifndef PLUGIN_PGDRIVER_H
#define PLUGIN_PGDRIVER_H

#include <QGenericPlugin>

class IPlainPGDriver;
namespace spdlog{ class logger;}
class InterfacePGDriver{
public:
    virtual ~InterfacePGDriver() = default;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger)=0;
    virtual std::shared_ptr<IPlainPGDriver> getIPlainPGDriverPtr()=0;
};

class plugin_pgdriver : public QGenericPlugin, InterfacePGDriver
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QGenericPluginFactoryInterface_iid FILE "pgdriver.json")

public:
    explicit plugin_pgdriver(QObject *parent = nullptr);

    typedef IPlainPGDriver* (*_ppgdriverf)() ;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger) override;
    virtual std::shared_ptr<IPlainPGDriver> getIPlainPGDriverPtr() override;
private:
    QObject *create(const QString &name, const QString &spec) override;
};

#endif // PLUGIN_PGDRIVER_H
