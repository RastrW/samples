#pragma once
#include "qmcr_api.h"
#include <string>


class IPlainRastr;
struct _object;

///@class Интерпретатор Python3
/// Подробности смотрите на:
/// [сайте проекта](https://docs.python.org/3/extending/extending.html#).
class QMCR_API PyHlp
{
public:
    enum class Result
    {
        Ok = 0,
        SyntaxError,
        RuntimeError,
        Error
    };

    PyHlp(const IPlainRastr& ipr);
    PyHlp(const PyHlp&)            = delete;
    PyHlp& operator=(const PyHlp&) = delete;

    ~PyHlp();

    std::string getErrorMessage() const noexcept;
    long        getErrorLine() const noexcept;
    long        getErrorOffset() const noexcept;

    Result run(const std::string_view macroText);
private:
    void captureError();
    bool initialize();

    _object* m_astraModule = nullptr;// PyObject*
    _object* m_rastrObject = nullptr;// PyObject*
    bool m_initialized = false;

    const IPlainRastr& m_ipr;

    std::string m_errorMessage;
    long m_errorLine = -1;
    long m_errorOffset = -1;

    static constexpr const char* k_scriptName = "rastr-py-embedded";
};
