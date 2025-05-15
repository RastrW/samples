

#include <QLibrary>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;
//include "C:\Projects\tfs\rastr\RastrWin\KC\IPlainTI.h"
#include "qti.h"
#include "IPlainRastrWrappers.h"

#include "pluginti.h"

template<class T>
class DestroyableTI
{
protected:
    T ptr_ = nullptr;
public:
    DestroyableTI(Destroyable<T>&& other) noexcept
    {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
    operator bool() const { return ptr_ != nullptr; }
    DestroyableTI(T ptr) : ptr_(ptr) {}
    virtual ~DestroyableTI() { if (ptr_) ptr_->Destroy(); }
    const T operator -> () const { return ptr_; }
    T operator -> () { return ptr_; }
};

void PluginTI::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}
std::shared_ptr<IPlainTI> PluginTI::getIPlainTIPtr(){
    std::shared_ptr<IPlainTI> shTIOut;
    try{
        const char* pch_name_comck_dll         {"COMCK"};
        const char* pch_name_plain_factory_fun {"PlainTIFactory"};
        QLibrary qlCOMCK{pch_name_comck_dll};
        if(qlCOMCK.load()){
            const QFunctionPointer pfn{ qlCOMCK.resolve(pch_name_plain_factory_fun) };
            if(pfn!=nullptr){
                _ptif fnFactory = reinterpret_cast<_ptif>(pfn);
                std::shared_ptr<IPlainTI> shTI {  (fnFactory)() };
                shTIOut.swap( shTI );
                spdlog::info("Get from [{}] functon: {}", pch_name_comck_dll, pch_name_plain_factory_fun);
            }else{
                spdlog::error("Not found functon: {} :: {}", pch_name_comck_dll, pch_name_plain_factory_fun);
            }
        }else{
            spdlog::error("Can't load: {}", pch_name_comck_dll);
        }
    }catch(const std::exception& ex){
        spdlog::error("Catch exception: {}", ex.what());
    }catch(...){
        spdlog::error("Catch unknown exception.");
    }
    return shTIOut;
}

