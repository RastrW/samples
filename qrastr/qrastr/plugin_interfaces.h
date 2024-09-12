#ifndef PLUGIN_INTERFACES_H
#define PLUGIN_INTERFACES_H

#include <QtPlugin>
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
class QWidget;
class QPainterPath;
class QPoint;
class QRect;
class QString;
QT_END_NAMESPACE

class IPlainRastr;
namespace spdlog{ class logger;}
class InterfaceRastr{
    public:
    virtual ~InterfaceRastr() = default;
    virtual void setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger)=0;
    virtual std::shared_ptr<IPlainRastr> getIPlainRastrPtr()=0;
};

QT_BEGIN_NAMESPACE
#define InterfaceRastr_iid "Rastr.Plugins.InterfaceRastr/1.0"
Q_DECLARE_INTERFACE( InterfaceRastr, InterfaceRastr_iid )
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACES_H
