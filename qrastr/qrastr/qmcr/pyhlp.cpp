#include "pyhlp.h"
//#include <Python.h>

#define Py_LIMITED_API  0x030A0000 //minimal version Python 3.10
#define PY_SSIZE_T_CLEAN

#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG 1
#else
    #include <Python.h>
#endif

PyHlp::PyHlp(const IPlainRastr& ipr)
    :IPlainRastr_(ipr)
{
}

PyHlp::~PyHlp()
{
    if(isInitialized_){
        Py_XDECREF(rastrPyObject_);
        Py_XDECREF(astraModule_);
        Py_FinalizeEx();
    }
}

void PyHlp::SetErrorMessage()
{
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    if(value){
        PyErr_NormalizeException(&type, &value, &traceback);
        const char* nameExeption = PyExceptionClass_Name(type);
        if(nameExeption){
            errorMessage_ = std::string(nameExeption).append(": ");
        }
        PyObject* pstr = PyObject_Str(value);
        if(pstr) {
            Py_ssize_t size;
            const char* msgError = PyUnicode_AsUTF8AndSize(pstr, &size);
            if(msgError != nullptr){
                errorMessage_.append(msgError, size);
            }
            Py_DECREF(pstr);
        }
        PyErr_Restore(type, value, traceback);
    }
    PyErr_Print();
}

bool PyHlp::Initialize()
{
    if(!Py_IsInitialized()){
        Py_InitializeEx(0);
        //PySys_SetPath(L"C:/projects/rastr/RastrWin/build/vs-Debug/astra/");
        //PySys_SetPath(L"R(C:\projects\rastr\RastrWin\build\vs-Debug\pyastra\)");
        //PySys_SetPath(L"C:/projects/rastr/RastrWin/build/vs-Debug/pyastra/)");
        //astraModule_ = PyImport_ImportModule("astra");
        PyObject *sys_path = PySys_GetObject("path");
#if(defined(_MSC_VER))
        PyList_Append(sys_path, PyUnicode_FromString("C:/projects/rastr/RastrWin/build/vs-Debug/pyastra/"));
#else
        PyList_Append(sys_path, PyUnicode_FromString("/home/ustas/projects/git_main/rastr/build-RastrWin-Desktop-Debug/pyastra"));
#endif
        //astraModule_ = PyImport_ImportModule("astra_py.cp310-win_amd64.pyd");
        astraModule_ = PyImport_ImportModule("astra_py");
        assert(nullptr != astraModule_);
        //astraModule_ = PyImport_ImportModule("sys");
        if(astraModule_ == nullptr){
            SetErrorMessage();
            Py_FinalizeEx();
            return false;
        }
        PyObject* py_rastr = PyObject_GetAttrString(astraModule_, "Rastr");
        if (py_rastr && PyCallable_Check(py_rastr)) {
            //auto ip = reinterpret_cast<IPlainRastr*>(rastrPtr_.operator->());
            //auto ptr = reinterpret_cast<uintptr_t>(ip);
            auto ptr = reinterpret_cast<uintptr_t>(&IPlainRastr_);
            PyObject* py_ptr = PyLong_FromUnsignedLongLong(ptr);
            PyObject* py_arg = PyTuple_Pack(1, py_ptr);
            Py_XDECREF(py_ptr);
            if (!py_arg) {
                Py_DECREF(py_rastr);
                return false;
            }
            rastrPyObject_ = PyObject_CallObject(py_rastr, py_arg);
            Py_XDECREF(py_arg);
            Py_XDECREF(py_rastr);
            if (PyErr_Occurred()){
                SetErrorMessage();
            }
        }else{
            if(PyErr_Occurred()){
                SetErrorMessage();
            }
            return false;
        }
    }
    isInitialized_ = true;
    return true;
}

PyHlp::enPythonResult PyHlp::Run(const std::string_view macroText)
{
    if (!Initialize()){
        return enPythonResult::Error;
    }
    PyObject* main_module = PyImport_AddModule("__main__");
    if (main_module == nullptr) {
        SetErrorMessage();
        return enPythonResult::Error;
    }
    PyObject* py_globals = PyModule_GetDict(main_module);
    if (!py_globals) {
        SetErrorMessage();
        return enPythonResult::Error;
    }
    if (rastrPyObject_) {
        PyMapping_SetItemString(py_globals, "astra", astraModule_);
        PyMapping_SetItemString(py_globals, "rastr", rastrPyObject_);
        PyObject* py_compiled = Py_CompileString(macroText.data(), "rastr-py-embeded", Py_file_input);
        if(py_compiled == nullptr){
            SetErrorMessage();
            return enPythonResult::SyntaxError;
        }
        PyObject* py_local = PyDict_New();
        PyObject* py_res = PyEval_EvalCode(py_compiled, py_globals, py_local);
        Py_XDECREF(py_res);
        Py_XDECREF(py_compiled);
        Py_XDECREF(py_local);
        PyMapping_DelItemString(py_globals, "astra");
        PyMapping_DelItemString(py_globals, "rastr");
        if (PyErr_Occurred()) {
            SetErrorMessage();
            return enPythonResult::RuntimeError;
        }
    }else{
        if (PyErr_Occurred()){
            SetErrorMessage();
            return enPythonResult::Error;
        }
    }
    return enPythonResult::Ok;
}
