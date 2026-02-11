#include <QLibrary>
#include <QCoreApplication>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;

#include "pluginti.h"

void PluginTI::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}
std::shared_ptr<IPlainTI> PluginTI::getIPlainTIPtr(){
    std::shared_ptr<IPlainTI> shTIOut;
    try{
        const char* pch_name_plain_factory_fun {"PlainTIFactory"};
        QString qstr_path_comck{QCoreApplication::applicationDirPath()};
        qstr_path_comck += "/plugins/COMCK";
        QLibrary qlCOMCK{qstr_path_comck};
        if(qlCOMCK.load()){
            const QFunctionPointer pfn{ qlCOMCK.resolve(pch_name_plain_factory_fun) };
            if(pfn!=nullptr){
                _ptif fnFactory = reinterpret_cast<_ptif>(pfn);
                std::shared_ptr<IPlainTI> shTI {  (fnFactory)() };
                shTIOut.swap( shTI );
                spdlog::info("Get from [{}] functon: {}",
                             qstr_path_comck.toStdString().c_str(),
                             pch_name_plain_factory_fun);
            }else{
                spdlog::error("Not found functon: {} :: {}",
                              qstr_path_comck.toStdString().c_str(),
                              pch_name_plain_factory_fun);
            }
        }else{
            spdlog::error("Can't load: {}", qstr_path_comck.toStdString().c_str());
        }
    }catch(const std::exception& ex){
        spdlog::error("Catch exception: {}", ex.what());
    }catch(...){
        spdlog::error("Catch unknown exception.");
    }
    return shTIOut;
}

