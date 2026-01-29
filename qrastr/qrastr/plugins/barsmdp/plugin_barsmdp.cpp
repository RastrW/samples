#include <QLibrary>
#include <QCoreApplication>

#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;
#include "qbarsmdp.h"
//#include "IPlainRastrWrappers.h"
#include <astra/IPlainRastrWrappers.h>

#include "plugin_barsmdp.h"

//template<class T>
/*class DestroyableKC
{
protected:
    T ptr_ = nullptr;
public:
    DestroyableKC(Destroyable<T>&& other) noexcept
    {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
    operator bool() const { return ptr_ != nullptr; }
    DestroyableKC(T ptr) : ptr_(ptr) {}
    virtual ~DestroyableKC() { if (ptr_) ptr_->Destroy(); }
    const T operator -> () const { return ptr_; }
    T operator -> () { return ptr_; }
};*/

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
                spdlog::info("Get from [{}] functon: {}", qstr_path_comck.toStdString().c_str(), pch_name_plain_factory_fun);
            }else{
                spdlog::error("Not found functon: {} :: {}", qstr_path_comck.toStdString().c_str(), pch_name_plain_factory_fun);
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

