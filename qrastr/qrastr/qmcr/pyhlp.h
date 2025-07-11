#ifndef PYHLP_H
#define PYHLP_H
#pragma once
//#include <QCoreApplication>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
class IPlainRastr;
struct _object;
class PyHlp
{
public:
     enum class enPythonResult
     {
         Ok = 0,
         SyntaxError,
         RuntimeError,
         Error
     };
     PyHlp(const IPlainRastr& ipr);
     ~PyHlp();
     void SetErrorMessage();
     std::string getErrorMessage() const noexcept;
     long        getErrorLine() const noexcept;
     long        getErrorOffset() const noexcept;
     bool Initialize();
     enPythonResult Run(const std::string_view macroText);
private:
     _object* astraModule_ = nullptr;//PyObject
     _object* rastrPyObject_ = nullptr;
     bool isInitialized_ = false;
     const IPlainRastr& IPlainRastr_;
     std::string errorMessage_;
     long nerrorLineno_ = -1;
     long nerrorOffset_ = -1;
     static constexpr const char* const pch_run_fname_= "rastr-py-embeded";
};//class PyHlp

#endif // PYHLP_H
