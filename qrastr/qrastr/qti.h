#ifndef QTI_H
#define QTI_H

#include <memory>
#include <QObject>
#include <comck/IPlainTI.h>
//#include "IPlainTI.h"
//#include "C:\Projects\tfs\rastr\RastrWin\build\install\include\comck\IPlainTI.h"

//#include "C:\Projects\tfs\rastr\RastrWin\comck\IPlainTI.h"
//#include "C:\projects\rastr\RastrWin\KC\IPlainTI.h"


class QTI  : public QObject
{
    Q_OBJECT
public:
    explicit  QTI(QObject *parent = nullptr);
    virtual   ~QTI() = default;
    typedef std::shared_ptr<IPlainTI> _sp_ti;
    void    setTI(const _sp_ti& sp_ti_in);
    _sp_ti  getTI() const;
    long Init() const;
    long RecalcDor() const;
    long UpdateTables() const;
    long CalcPTI() const;
    long DobavPTI() const;
    long FiltrTI() const;
private:
    _sp_ti sp_ti_;
};

#endif // QTI_H
