#ifndef QBARSMDP_H
#define QBARSMDP_H

#include <memory>
#include <QObject>
#include "IPlainBarsMDP.h"
//#include <comck/IPlainBarsMDP.h>
//#include "C:\Projects\tfs\rastr\RastrWin\build\install\include\comck\IPlainBarsMDP.h"

class QBarsMDP  : public QObject
{
    Q_OBJECT
public:
    explicit  QBarsMDP(QObject *parent = nullptr);
    virtual   ~QBarsMDP() = default;
    typedef std::shared_ptr<IPlainBarsMDP> _sp_barsmdp;
    void    setBarsMDP(const _sp_barsmdp& sp_barsmdp_in);
    _sp_barsmdp  getBarsMDP() const;
    long Init(std::string _Sech);
    long UpdateMDPFields();
    long UpdateAUTOFields();
private:
    _sp_barsmdp sp_barsmdp_;
};


#endif // QBARSMDP_H
