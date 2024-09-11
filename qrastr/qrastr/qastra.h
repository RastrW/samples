#ifndef QASTRA_H
#define QASTRA_H

#include <QObject>
#include "IPlainRastr.h"
//class IPlainRastr;
class EventSink;
class QAstra
    : public QObject{
    Q_OBJECT
public:
    typedef std::shared_ptr<IPlainRastr> _sp_rastr;
    explicit QAstra(QObject *parent = nullptr);
    virtual ~QAstra() = default;
    void setRastr(_sp_rastr sp_rastr_in);
    void LoadFile( eLoadCode LoadCode, const std::string_view& FilePath, const std::string_view& TemplatePath );
    eASTCode Rgm(const std::string_view& parameters = {});
signals:
private:
    _sp_rastr sp_rastr_;
    std::unique_ptr<EventSink> up_EventSink_;
};

#endif // QASTRA_H
