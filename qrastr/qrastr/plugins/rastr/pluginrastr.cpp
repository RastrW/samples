// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
//#if(QT_VERSION > QT_VERSION_CHECK(5, 16, 0))
    #include <QLibrary>
//#else
//#endif
#include <QCoreApplication>
#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"
#include "pluginrastr.h"

void PluginRastr::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}

std::shared_ptr<IPlainRastr> PluginRastr::getIPlainRastrPtr(){
    std::shared_ptr<IPlainRastr> shRastrOut;
    try{
        const char* pch_name_plain_factory_fun {"PlainRastrFactory"};
        //const char* pch_name_astra_dll         {"astra"};
        //const char* pch_name_astra_dll         {R"(C:\projects\git_web\samples\qrastr\Debug\plugins\astra.dll)"};
        QString qstr_path_astra{QCoreApplication::applicationDirPath()};
        qstr_path_astra += "/plugins/astra";
        //qstr_path_astra += "/plugins/astra_py.cp310-win_amd64.pyd";
        //QLibrary qlRastr{pch_name_astra_dll};
        QLibrary qlRastr{qstr_path_astra};
        if(qlRastr.load()){
            const QFunctionPointer pfn{ qlRastr.resolve(pch_name_plain_factory_fun) };
            if(pfn!=nullptr){
                _prf fnFactory = reinterpret_cast<_prf>(pfn);
                std::shared_ptr<IPlainRastr> shRastr {  (fnFactory)() };
                shRastrOut.swap( shRastr );
                //spdlog::info("Get from [{}] functon: {}", pch_name_astra_dll, pch_name_plain_factory_fun);
                spdlog::info("Get from [{}] functon: {}", qstr_path_astra.toStdString(), pch_name_plain_factory_fun);

            }else{
                //spdlog::error("Not found functon: {} :: {}", pch_name_astra_dll, pch_name_plain_factory_fun);
                spdlog::error("Not found functon: {} :: {}", qstr_path_astra.toStdString(), pch_name_plain_factory_fun);
            }
        }else{
            //spdlog::error("Can't load: {}", pch_name_astra_dll);
            spdlog::error("Can't load: {}", qstr_path_astra.toStdString());
        }
    }catch(const std::exception& ex){
        spdlog::error("Catch exception: {}", ex.what());
    }catch(...){
        spdlog::error("Catch unknown exception.");
    }
    return shRastrOut;
}


#if(!defined(_MSC_VER))
    int main(int argc, char *argv[]){
        return 1;
    }
#endif
