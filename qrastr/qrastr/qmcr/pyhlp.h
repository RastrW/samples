#ifndef PYHLP_H
#define PYHLP_H
#pragma once

#include <string>

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
     bool Initialize();
     enPythonResult Run(const std::string_view macroText);
private:
     _object* astraModule_ = nullptr;
     _object* rastrPyObject_ = nullptr;
     std::string errorMessage_;
     bool isInitialized_ = false;
     const IPlainRastr& IPlainRastr_;
};//class PyHlp

#endif // PYHLP_H
