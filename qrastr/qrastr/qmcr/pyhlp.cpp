#include "pyhlp.h"

#define Py_LIMITED_API  0x030A0000 //minimal version Python 3.10
#define PY_SSIZE_T_CLEAN

#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG 1
#else
    #include <Python.h>
#endif

#include <QDebug>

namespace PyUtils
{
    struct PyObjRaii
    {
        PyObjRaii() = delete;
        PyObjRaii( PyObject* pPyObj )
        {
            pPyObj_ = pPyObj;
        }

        auto operator=( PyObject*  pPyObj) -> PyObjRaii&
        {
            pPyObj_ = pPyObj;
            return *this;
        }

        operator PyObjRaii()
        {
            return *this;
        }

        virtual ~PyObjRaii(){
            if(pPyObj_ != nullptr){
                assert( pPyObj_->ob_refcnt > 0);
                qDebug()<<"pPyObj_->ob_refcnt== "<<pPyObj_->ob_refcnt<<"\n";
                Py_XDECREF(pPyObj_);
            }
        }

        operator PyObject*()
        {
            return pPyObj_;
        }

    private:
        PyObject* pPyObj_ = nullptr;
    };

    std::string PyObjToStr(PyObject* pPyObj)
    {
        std::string str = "__err__";
        if(nullptr != pPyObj){
            PyObjRaii str_exc_value = PyObject_Repr(pPyObj);
            if(nullptr != str_exc_value){
                PyObjRaii pyExcValueStr = PyUnicode_AsEncodedString( str_exc_value, "utf-8", "strict" );
                if(nullptr != pyExcValueStr){
                    str = PyBytes_AsString( pyExcValueStr );
                }
            }
        }
        return str;
    }
}

#include <sstream>
//pyerrors.h
//https://habr.com/ru/articles/167261/
//https://github.com/shira-374/rstpad // qt python editor
void PyHlp::SetErrorMessage()
{
    errorMessage_.clear();
    nerrorLineno_ = -1;
    nerrorOffset_ = -1;
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch( &ptype, &pvalue, &ptraceback );
    if(pvalue){
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        const char* nameExeption = PyExceptionClass_Name(ptype);
        if(nameExeption){
            //errorMessage_ = std::string(nameExeption).append(": ");
        }
        const std::string str_pvalue = PyUtils::PyObjToStr(pvalue);
        errorMessage_ = str_pvalue;
        if(PyExc_SyntaxError == ptype){
            PyUtils::PyObjRaii pyFileName = PyObject_GetAttrString(pvalue, "filename");
            PyUtils::PyObjRaii pyLineNo   = PyObject_GetAttrString(pvalue, "lineno");
            nerrorLineno_ = PyLong_AsLong(pyLineNo);
            PyUtils::PyObjRaii pyOffset   = PyObject_GetAttrString(pvalue, "offset");
            nerrorOffset_ = PyLong_AsLong(pyOffset);
            std::string str_filename = PyUtils::PyObjToStr(pyFileName);
            std::string str_lineno   = PyUtils::PyObjToStr(pyLineNo);
            std::string str_offset   = PyUtils::PyObjToStr(pyOffset);
        }else if(PyExc_ModuleNotFoundError == ptype){
            PyUtils::PyObjRaii pyFileName = PyObject_GetAttrString(pvalue, "name");
            PyUtils::PyObjRaii pyPath     = PyObject_GetAttrString(pvalue, "path");
            std::string str_filename      = PyUtils::PyObjToStr(pyFileName);
            std::string str_path     = PyUtils::PyObjToStr(pyPath);
        }else if(PyExc_ImportError == ptype){
            //printf("hell owrorld 33\n");
        }
    }
    PyErr_Restore(ptype, pvalue, ptraceback);
    PyErr_Print();
    return;

    // examples ////////////////////////////
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    if(value){
        PyErr_NormalizeException(&type, &value, &traceback);

        //0
        //PyObject *str_exc_value = PyObject_Repr(value);
        //PyObject *pyExcValueStr = PyUnicode_AsEncodedString(str_exc_value, "utf-8", "Error ~");
        //std::string errorMessage =  PyBytes_AsString(pyExcValueStr) ;
        const std::string errorMessage =  PyUtils::PyObjToStr(value);

        //1 //https://stackoverflow.com/questions/16733425/how-to-retrieve-filename-and-lineno-attribute-of-syntaxerror
        {
            //PyObject *ptype = NULL, *pvalue = NULL, *ptraceback = NULL;
            //PyErr_Fetch(&ptype,&pvalue,&ptraceback);
            //PyErr_NormalizeException(&ptype,&pvalue,&ptraceback);

            char *msg;
            char *file;
            int line;
            int offset;
            char *text;

            //int res = PyArg_ParseTuple(pvalue,"s(siis)",&msg,&file,&line,&offset,&text);
            int res = PyArg_ParseTuple(value,"s(siis)",&msg,&file,&line,&offset,&text);

            //PyObject* file_name = PyObject_GetAttrString(pvalue,"filename");
            PyObject* file_name = PyObject_GetAttrString(value,"filename");
            PyObject* file_name_str = PyObject_Str(file_name);
            PyObject* file_name_unicode = PyUnicode_AsEncodedString(file_name_str,"utf-8", "Error");
            char *actual_file_name = PyBytes_AsString(file_name_unicode);

            //PyObject* line_no = PyObject_GetAttrString(pvalue,"lineno");
            PyObject* line_no = PyObject_GetAttrString(value,"lineno");
            PyObject* line_no_str = PyObject_Str(line_no);
            PyObject* line_no_unicode = PyUnicode_AsEncodedString(line_no_str,"utf-8", "Error");
            char *actual_line_no = PyBytes_AsString(line_no_unicode);


            PyObject* lasti_no = PyObject_GetAttrString(value,"offset");
            PyObject* lasti_no_str = PyObject_Str(lasti_no);
            PyObject* lasti_no_unicode = PyUnicode_AsEncodedString(lasti_no_str,"utf-8", "Error");
            char *actual_lasti = PyBytes_AsString(lasti_no_unicode);
            printf("done");
        }

        //1.5
        if (type == PyExc_SyntaxError) {
            PyObject *pyFileName = PyObject_GetAttrString(value, "filename");
            PyObject *pyLineNo = PyObject_GetAttrString(value, "lineno");
            std::string LineNo = PyUtils::PyObjToStr(pyLineNo);
            if (pyFileName) {
                std::string fileNameWithFullPath =  PyUtils::PyObjToStr(pyFileName);
                Py_DECREF(pyFileName);
                // valueStr.erase(fileNameStart, fileNameEnd - fileNameStart);
                // valueStr.insert(fileNameStart, fileNameWithFullPath);
                // fileNameEnd += fileNameWithFullPath.size() - (fileNameEnd - fileNameStart);
            }
        }

        //2
        PyObject * traceback_module = PyImport_ImportModule("traceback");
        if(nullptr != traceback_module){
            std::stringstream  sstr;
            PyObject *  format_exception = PyObject_GetAttrString(traceback_module, "format_exception");
            PyObject *  args = PyTuple_New(3);
            PyObject*  arg0 = type ? static_cast<PyObject*>(type) : Py_None;
            Py_IncRef(arg0);
            PyObject*  arg1 = value ? static_cast<PyObject*>(value) : Py_None;
            Py_IncRef(arg1);
            PyObject*  arg2 = traceback ? static_cast<PyObject*>(traceback) : Py_None;
            Py_IncRef(arg2);
            PyTuple_SetItem(args, 0, arg0);
            PyTuple_SetItem(args, 1, arg1);
            PyTuple_SetItem(args, 2, arg2);
            PyObject*  lst = PyObject_Call(format_exception, args, NULL);
            sstr << std::endl << std::endl;
            for (size_t i = 0; i < PyList_Size(lst); ++i){
               PyObject* item = PyList_GetItem(lst, i);
               std::string pretty = PyBytes_AsString(PyUnicode_AsASCIIString(item));
               sstr <<pretty << "\n";
               //sstr << std::string(convert_from_python(item)) << std::endl;
            }
            //throw std::exception(sstr.str().c_str());
            throw std::runtime_error(sstr.str().c_str());
        }

        const char* nameExeption = PyExceptionClass_Name(type);
        if(nameExeption){
            errorMessage_ = std::string(nameExeption).append(": ");
        }
        PyObject* pstr = PyObject_Str(value);
        //int line = PyLong_AsLong(PyObject_GetAttrString(PyObject_GetAttrString(value, "tb_frame"), "f_lineno"));
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

std::string PyHlp::getErrorMessage() const noexcept
{
    return errorMessage_;
}

long PyHlp::getErrorLine() const noexcept
{
    return nerrorLineno_;
}

long PyHlp::getErrorOffset() const noexcept
{
    return nerrorOffset_;
}

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

bool PyHlp::Initialize()
{
    int nRes = 0;
    if(!Py_IsInitialized()){
        Py_InitializeEx(0);
        //PyUtils::PyObjRaii sys_path = PySys_GetObject("path"); assert(nullptr != sys_path);
        PyObject* sys_path = PySys_GetObject("path"); assert(nullptr != sys_path);//Borrowed reference!
        std::string str_path1 = PyUtils::PyObjToStr(sys_path);
#if(defined(_MSC_VER))
        //nRes = PyList_Append(sys_path, PyUnicode_FromString(R"(C:/projects/rastr/RastrWin/build/vs-Debug/pyastra/)")); assert(0 == nRes);
        nRes = PyList_Append(sys_path, PyUnicode_FromString(R"(C:\projects\rastr\RastrWin\build\vs-Debug\pyastra\)")); assert(0 == nRes);
#else
        nRes = PyList_Append(sys_path, PyUnicode_FromString("/home/ustas/projects/git_main/rastr/build-RastrWin-Desktop-Debug/pyastra")); assert(0 == nRes);
#endif
        //PyUtils::PyObjRaii sys_path2 = PySys_GetObject("path"); assert(nullptr != sys_path);
        PyObject* sys_path2 = PySys_GetObject("path"); assert(nullptr != sys_path);
        std::string str_path2 = PyUtils::PyObjToStr(sys_path2);
        //astraModule_ = PyImport_ImportModule("astra_py.cp310-win_amd64.pyd");
        astraModule_ = PyImport_ImportModule("astra_py");
        assert(nullptr != astraModule_);
        if(nullptr == astraModule_){
            SetErrorMessage();
            Py_FinalizeEx();
            return false;
        }
        PyUtils::PyObjRaii py_rastr = PyObject_GetAttrString(astraModule_, "Rastr");
        if (py_rastr && PyCallable_Check(py_rastr)) {
            const uintptr_t ptr = reinterpret_cast<uintptr_t>(&IPlainRastr_);
            PyObject* py_ptr = PyLong_FromUnsignedLongLong(ptr);
            PyUtils::PyObjRaii py_arg = PyTuple_Pack(1, py_ptr);
            Py_XDECREF(py_ptr);
            if (!py_arg) {
                return false;
            }
            rastrPyObject_ = PyObject_CallObject(py_rastr, py_arg);
            if (PyErr_Occurred()){
                SetErrorMessage();
                return false;
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
    int nRes = 0;
    if (!Initialize()){
        return enPythonResult::Error;
    }
    PyObject* main_module = PyImport_AddModule("__main__"); // return borrowed reference
    if(nullptr == main_module){
        SetErrorMessage();
        return enPythonResult::Error;
    }
    PyObject* py_globals = PyModule_GetDict(main_module); // return borrowed reference
    if(nullptr == py_globals){
        SetErrorMessage();
        return enPythonResult::Error;
    }
    if(rastrPyObject_){
        nRes = PyMapping_SetItemString(py_globals, "astra", astraModule_);assert(0 == nRes);
        nRes = PyMapping_SetItemString(py_globals, "rastr", rastrPyObject_); assert(0 == nRes);
        PyUtils::PyObjRaii py_compiled = Py_CompileString( macroText.data(), pch_run_fname_, Py_file_input );
        if(py_compiled == nullptr){
            SetErrorMessage();
            return enPythonResult::SyntaxError;
        }
        PyUtils::PyObjRaii py_local = PyDict_New();
        PyUtils::PyObjRaii py_res = PyEval_EvalCode(py_compiled, py_globals, py_local);
        nRes = PyMapping_DelItemString(py_globals, "astra"); assert(-1 != nRes);
        nRes = PyMapping_DelItemString(py_globals, "rastr"); assert(-1 != nRes);
        if(PyErr_Occurred()){
            SetErrorMessage();
            return enPythonResult::RuntimeError;
        }
    }else{
        if(PyErr_Occurred()){
            SetErrorMessage();
            return enPythonResult::Error;
        }
    }
    return enPythonResult::Ok;
}
