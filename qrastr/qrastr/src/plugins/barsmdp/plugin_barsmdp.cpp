#include <QLibrary>
#include <QDir>
#include <QCoreApplication>
#include <spdlog/spdlog.h>
using WrapperExceptionType = std::runtime_error;
#include <astra/IPlainRastrWrappers.h>

#include "plugin_barsmdp.h"
#include "IPlainBarsMDP.h"

void PluginBarsMDP::setLoggerPtr(std::shared_ptr<spdlog::logger> spLoger){
    spdlog::set_default_logger(spLoger);
    spdlog::info("RastrPlugin get logger");
}

std::string getLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return "No error message has been recorded";

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

std::shared_ptr<IPlainBarsMDP> PluginBarsMDP::getIPlainBarsMDPPtr(){
    std::shared_ptr<IPlainBarsMDP> shBarsMDPOut;
    try{
        const char* pch_name_plain_factory_fun {"PlainBarsMDPFactory"};
        QDir dir(QCoreApplication::applicationDirPath());
        QString qstr_path_comck = dir.filePath("plugins/COMCK.dll");

        std::string s = qstr_path_comck.toStdString();
        std::wstring wstr = std::wstring(s.begin(), s.end());
        LPCWSTR result = wstr.c_str(); // Use immediately
        HMODULE libraryHandle = ::LoadLibrary(result);
        if ( libraryHandle == 0 )
        {
            const DWORD errorCode = ::GetLastError();
            qDebug() << "error: code(" << errorCode << "), description(" << getLastErrorAsString() << ')';
        }



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
            spdlog::error("Can't load: {} | error: {}",
                          qstr_path_comck.toStdString(),
                          qlCOMCK.errorString().toStdString());
        }
    }catch(const std::exception& ex){
        spdlog::error("Catch exception: {}", ex.what());
    }catch(...){
        spdlog::error("Catch unknown exception.");
    }
    return shBarsMDPOut;
}

