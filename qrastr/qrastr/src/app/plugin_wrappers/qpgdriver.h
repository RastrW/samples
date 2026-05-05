#pragma once

//#include <memory>
#include <QObject>
#include <comck/IPlainPGDriver.h>


///@class Обёртка для плагина PGDriver
class QPGDriver : public QObject
{
    Q_OBJECT
public:
    explicit QPGDriver(QObject *parent = nullptr);
    virtual   ~QPGDriver() = default;
    typedef std::shared_ptr<IPlainPGDriver> _sp_pgdriver;
    void    setPGDriver(const _sp_pgdriver& sp_pgdriver_in);
    _sp_pgdriver  getPGDriver() const;
    int Init();
    int Connect();
    int Table_R2SQL(std::string Table, std::string cols, std::string Viborka);
    int Table_SQL2R(std::string Table, std::string cols, std::string Viborka);
    // Прочитать все из растр (текущая рабочая область) и записать в БД SQL
    int All_R2SQL(std::string TemplateName);
    // Прочитать все из БД SQL в растр (текущая рабочая область)
    int All_SQL2R(std::string TemplateName);
private:
    _sp_pgdriver sp_pgdriver_;
};


