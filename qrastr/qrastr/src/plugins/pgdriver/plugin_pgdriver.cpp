#include <QLibrary>
#include <QCoreApplication>
#include <QDir>
#include <spdlog/spdlog.h>
#include "plugin_pgdriver.h"
#include "IPlainPGDriver.h"

void plugin_pgdriver::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}

std::shared_ptr<IPlainPGDriver> plugin_pgdriver::getIPlainPGDriverPtr(){
    std::shared_ptr<IPlainPGDriver> shPGDriverOut;
    try{
        const char* pch_name_plain_factory_fun {"PlainPGDriverFactory"};
        QDir dir(QCoreApplication::applicationDirPath());
        QString qstr_path_comck = dir.filePath("plugins/COMCK.dll");
        QLibrary qlCOMCK{qstr_path_comck};
        if(qlCOMCK.load()){
            const QFunctionPointer pfn{ qlCOMCK.resolve(pch_name_plain_factory_fun) };
            if(pfn!=nullptr){
                _ppgdriverf fnFactory = reinterpret_cast<_ppgdriverf>(pfn);
                std::shared_ptr<IPlainPGDriver> shPGDriver {  (fnFactory)() };
                shPGDriverOut.swap( shPGDriver );
                spdlog::info("Get from [{}] functon: {}",
                             qstr_path_comck.toStdString().c_str(),
                             pch_name_plain_factory_fun);
            }else{
                spdlog::error("Not found functon: {} :: {}",
                              qstr_path_comck.toStdString().c_str(),
                              pch_name_plain_factory_fun);
            }
        }else{
            spdlog::error("Can't load: {} | error: {}",
                          qstr_path_comck.toStdString(),
                          qlCOMCK.errorString().toStdString());
        }
    }catch(const std::exception& ex){
        spdlog::error("Catch exception: {}", ex.what());
    }catch(...){
        spdlog::error("Catch unknown exception.");
    }
    return shPGDriverOut;
}
