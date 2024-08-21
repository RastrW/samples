#ifndef QASTRA_H
#define QASTRA_H
#include <QObject>
class IPlainRastr;
class qAstra : public QObject{
    Q_OBJECT
public:
    typedef IPlainRastr* (__cdecl* _prf)() ;
    explicit qAstra(QObject *parent = nullptr);
    virtual ~qAstra() = default;
    int tst_iplainrastr() const;
signals:

};//class qAstra

#endif // QASTRA_H
