#include <QLibrary>
#include <QCoreApplication>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>

#include "plugin_barsmdp.h"

void PluginBarsMDP::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}
std::shared_ptr<IPlainBarsMDP> PluginBarsMDP::getIPlainBarsMDPPtr(){
    std::shared_ptr<IPlainBarsMDP> shBarsMDPOut;
    try{
        const char* pch_name_plain_factory_fun {"PlainBarsMDPFactory"};
        QString qstr_path_comck{QCoreApplication::applicationDirPath()};
        qstr_path_comck += "/plugins/COMCK";
        QLibrary qlCOMCK{qstr_path_comck};
        if(qlCOMCK.load()){
            const QFunctionPointer pfn{ qlCOMCK.resolve(pch_name_plain_factory_fun) };
            if(pfn!=nullptr){
                _pbarsmdpf fnFactory = reinterpret_cast<_pbarsmdpf>(pfn);
                std::shared_ptr<IPlainBarsMDP> shBarsMDP {  (fnFactory)() };
                shBarsMDPOut.swap( shBarsMDP );
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
    return shBarsMDPOut;
}

