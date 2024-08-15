#include "py_fun.h"

#ifdef PY_ASTRA

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/embed.h> // everything needed for embedding

namespace py = pybind11;
using namespace pybind11::literals;

/// Python 2.7/3.x compatible version of `PyImport_AppendInittab` and error checks.
//! ustas: replaced call to "pybind11_fail" to return
struct safe_embedded_module {
    using init_t = PyObject *(*) ();
    safe_embedded_module(const char *name, init_t init) {
        if (Py_IsInitialized() != 0) {
            //! ustas: //pybind11_fail("Can't add new modules after the interpreter has been initialized");
            //assert(0);
            return;
        }
        auto result = PyImport_AppendInittab(name, init);
        if (result == -1) {
            ::pybind11::pybind11_fail("Insufficient memory to add a new module");
        }
    }
};


/** \rst
Add a new module to the table of builtins for the interpreter. Must be
defined in global scope. The first macro parameter is the name of the
module (without quotes). The second parameter is the variable which will
be used as the interface to add functions and classes to the module.

.. code-block:: cpp

    PYBIND11_EMBEDDED_MODULE(example, m) {
        // ... initialize functions and classes here
        m.def("foo", []() {
            return "Hello, World!";
        });
    }
\endrst */
//! ustas: replaced call to my "safe_" function of embedded module (embedded_module -> safe_embedded_module)
#define safe_PYBIND11_EMBEDDED_MODULE(name, variable)                                             \
    static ::pybind11::module_::module_def PYBIND11_CONCAT(pybind11_module_def_, name);           \
    static void PYBIND11_CONCAT(pybind11_init_, name)(::pybind11::module_ &);                     \
    static PyObject PYBIND11_CONCAT(*pybind11_init_wrapper_, name)() {                            \
        auto m = ::pybind11::module_::create_extension_module(                                    \
            PYBIND11_TOSTRING(name), nullptr, &PYBIND11_CONCAT(pybind11_module_def_, name));      \
        try {                                                                                     \
            PYBIND11_CONCAT(pybind11_init_, name)(m);                                             \
            return m.ptr();                                                                       \
        }                                                                                         \
        PYBIND11_CATCH_INIT_EXCEPTIONS                                                            \
    }                                                                                             \
    PYBIND11_EMBEDDED_MODULE_IMPL(name)                                                           \
    safe_embedded_module PYBIND11_CONCAT(pybind11_module_, name)(                  \
        PYBIND11_TOSTRING(name), PYBIND11_CONCAT(pybind11_init_impl_, name));                     \
    void PYBIND11_CONCAT(pybind11_init_, name)(::pybind11::module_                                \
                                               & variable) // NOLINT(bugprone-macro-parentheses)



int add(int i, int j) {
    return i + j;
}

struct Pet {
    Pet(const std::string &name)
        : name_(name) {
    }
    void setName(const std::string &name) {
        name_ = name;
    }
    const std::string &getName() const {
        return name_;
    }
    void bark(){
        py::print("[%s].bark!",name_.c_str());
    }
    std::string name_;
};

template< typename _Class >
void add_Pet_Func( _Class&& m_in ){
//py::module_
//void add_Pet_Func( py::module_&& m_in ){
    m_in.def( "setName", &Pet::setName );
    m_in.def( "getName", &Pet::getName );
    m_in.def( "bark",    &Pet::bark    );
};

safe_PYBIND11_EMBEDDED_MODULE(PetsTst, m) {
    add_Pet_Func(
        py::class_< Pet, std::shared_ptr<Pet> >(m, "Pet") //use existed Pet object
    );
/*    pybind11::detail::generic_type d = py::class_< Pet, std::shared_ptr<Pet> >(m, "Pet");
    py::class_< Pet, std::shared_ptr<Pet> >(m, "Pet")
            .def( "setName", &Pet::setName )
            .def( "getName", &Pet::getName )
            .def( "bark",    &Pet::bark    ); //use existed Pet object
*/
  //  add_Pet_Func(
    //);
}

//no in lin! PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> exc_storage;

#include <frameobject.h>

typedef std::vector<std::string> wxArrayString;
wxArrayString PyArrayStringToWx( PyObject* aArrayString )
{
    wxArrayString   ret;

    if( !aArrayString )
        return ret;

    int list_size = PyList_Size( aArrayString );

    for( int n = 0; n < list_size; n++ )
    {
        PyObject* element = PyList_GetItem( aArrayString, n );

        if( element )
        {
            const char* str_res = nullptr;
            PyObject* temp_bytes = PyUnicode_AsEncodedString( element, "UTF-8", "strict" );

            if( temp_bytes != nullptr )
            {
                str_res = PyBytes_AS_STRING( temp_bytes );
                //ret.Add( From_UTF8( str_res ), 1 );
                ret.emplace_back(str_res );
                Py_DECREF( temp_bytes );
            }
            else
            {
                assert(!"xz");
                //wxLogMessage( wxS( "cannot encode Unicode python string" ) );
            }
        }
    }

    return ret;
}

#include <regex>

std::shared_ptr<Pet> pet = std::make_shared<Pet>("TestttPet");
int g_n_num_imp = 0;

long EmbPyRunMacro( const std::string& str_py_macro ){
    py::scoped_interpreter guard{};
    try{
        //py::scoped_interpreter guard{}; // start the interpreter and keep it alive
        //py::exec("import pdb");
        //py::exec("pdb.set_trace()");
        //py::exec("breakpoint()");
     //   py::exec("import sys");
        /*
        std::shared_ptr<Pet> pet = std::make_shared<Pet>("TestttPet");

*/
        //if(g_n_num_imp == 0){
            g_n_num_imp++;
            auto pets_mod = py::module::import("PetsTst");
            py::globals()["pet"] = py::cast(pet);
        //}
        py::exec(str_py_macro);

        /*
        py::exec(R"(
print('hello wold')
print('hello wold2')
sys.os
pet.setName1('testPet'))");
        py::exec("pet.bark()");
        */
    }catch( const py::error_already_set& e ){
        const char* pch = e.what();
        if(nullptr != pch){
            const std::string str_excp{pch};
            const char* pch_start  {"<string>("};
            const char* pch_finish {")"};
            const std::string::size_type st_start = str_excp.find(pch_start);
            if(std::string::npos != st_start){
                const std::string::size_type st_line_num_start  = st_start+std::strlen(pch_start);
                const std::string::size_type st_line_num_finish = str_excp.find(pch_finish, st_line_num_start);
                if(std::string::npos != st_line_num_finish){
                    if(st_line_num_finish > st_line_num_start){
                        const std::string str_line_num {str_excp.substr(st_line_num_start, st_line_num_finish-st_line_num_start)};
                        const long n_line_num = std::stol(str_line_num);
                    }
                }
            }
printf("dfdf");

        }
        return -1;

        /*
        //https://gitlab.com/kicad/code/kicad/-/blob/master/scripting/python_scripting.cpp
        PyException_SetTraceback( e.value().ptr(), e.trace().ptr() );//!ustas not understend for what this, disable

        //PyObject* objectsRepresentation = PyObject_Repr(e.trace().ptr());
        PyObject* objectsRepresentation = PyObject_Repr(e.value().ptr());
        PyObject* pyStr = PyUnicode_AsEncodedString(objectsRepresentation, "utf-8", "Error ~");
        const char *strEx = PyBytes_AS_STRING(pyStr);
         //const char* s = PyString_AsString(objectsRepresentation);

        PyObject* tracebackModuleString = PyUnicode_FromString( "traceback" );
        PyObject* tracebackModule = PyImport_Import( tracebackModuleString );
        Py_DECREF( tracebackModuleString );

        PyObject* formatException = PyObject_GetAttrString( tracebackModule,
                                                      "format_exception" );
        Py_DECREF( tracebackModule );
        //PyObject* args = Py_BuildValue( "(O,O,O)", e.type(), e.value(), e.trace() );
        //PyObject* args = Py_BuildValue( "(O,O,O)", e.value(), e.trace(), e.type() );
        PyObject* args = Py_BuildValue( "(O,O,O)", e.type(), e.value(), e.trace() );

       // wxArrayString res1 = PyArrayStringToWx( args );
        PyObject* result = PyObject_CallObject( formatException, args );
        Py_XDECREF( formatException );
        Py_XDECREF( args );
        wxArrayString res = PyArrayStringToWx( result );
        Py_XDECREF( result );
        PyErr_Clear();

        return -1;

*/

/*
        e.restore();
        PyObject *ptype = NULL, *pvalue = NULL, *ptraceback = NULL;
        PyErr_Fetch(&ptype,&pvalue,&ptraceback);
        //PyErr_Fetch(&ptype,&pvalue,&ptraceback);
        PyErr_NormalizeException(&ptype,&pvalue,&ptraceback);
//PyUnicode_FromString
        //char *pStrErrorMessage = PyString_AsString(pvalue);
        PyObject* str_exc_type = PyObject_Repr(ptype); //Now a unicode        object
        PyObject* pyStr = PyUnicode_AsEncodedString(str_exc_type, "utf-8", "Error ~");
        const char *strExcType = PyBytes_AS_STRING(pyStr);

        PyTracebackObject* traceback = reinterpret_cast<PyTracebackObject*>(ptraceback); //get_the_traceback();

         int line0 = traceback->tb_lineno;
     //   const char* filename = PyString_AsString(traceback->tb_frame->f_code->co_filename);

        {
             //https://gitlab.com/kicad/code/kicad/-/blob/master/scripting/python_scripting.cpp
             PyObject*   type = ptype;
              PyObject*   value = pvalue;
              PyObject*   traceback = ptraceback;

              //PyErr_Fetch( &type, &value, &traceback );

              //PyErr_NormalizeException( &type, &value, &traceback );

              if( traceback == nullptr )
              {
                  traceback = Py_None;
                  Py_INCREF( traceback );
              }
              PyException_SetTraceback( value, traceback );
              PyObject* tracebackModuleString = PyUnicode_FromString( "traceback" );
              PyObject* tracebackModule = PyImport_Import( tracebackModuleString );
              Py_DECREF( tracebackModuleString );
              PyObject* formatException = PyObject_GetAttrString( tracebackModule,
                                                                  "format_exception" );
              Py_DECREF( tracebackModule );
               PyObject* args = Py_BuildValue( "(O,O,O)", type, value, traceback );
               PyObject* result = PyObject_CallObject( formatException, args );
               Py_XDECREF( formatException );
               Py_XDECREF( args );
               Py_XDECREF( type );
               Py_XDECREF( value );
               Py_XDECREF( traceback );

               wxArrayString res2 = PyArrayStringToWx( result );

               PyErr_Clear();
         }
*/

/*
        PyThreadState *tstate = PyThreadState_GET();
        if (NULL != tstate && NULL != tstate ->cframe) {
            //PyFrameObject *frame = tstate->cframe;
            _PyCFrame *frame = tstate->cframe;

            printf("Python stack trace:\n");
            while (NULL != frame) {
                // int line = frame->f_lineno;

                 //frame->f_lineno will not always return the correct line number
                 //you need to call PyCode_Addr2Line().

                int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
                const char *filename = PyString_AsString(frame->f_code->co_filename);
                const char *funcname = PyString_AsString(frame->f_code->co_name);
                printf("    %s(%d): %s\n", filename, line, funcname);
                frame = frame->f_back;
            }
        }
*/
/*
        char *msg;
        char *file;
        int line;
        int offset;
        char *text;

        int res4 = PyArg_ParseTuple(pvalue,"s(siis)",&msg,&file,&line,&offset,&text);

        PyObject* file_name = PyObject_GetAttrString(pvalue,"filename");
        PyObject* file_name_str = PyObject_Str(file_name);
        PyObject* file_name_unicode = PyUnicode_AsEncodedString(file_name_str,"utf-8", "Error");
        char *actual_file_name = PyBytes_AsString(file_name_unicode);

        PyObject* line_no = PyObject_GetAttrString(pvalue,"lineno");
        PyObject* line_no_str = PyObject_Str(line_no);
        PyObject* line_no_unicode = PyUnicode_AsEncodedString(line_no_str,"utf-8", "Error");
        char *actual_line_no = PyBytes_AsString(line_no_unicode);

        printf("done");

        if (e.matches(PyExc_FileNotFoundError)) {
            py::print("missing.txt not found");
        } else if (e.matches(PyExc_PermissionError)) {
            py::print("missing.txt found but not accessible");
        }else if(e.matches( PyExc_SyntaxError)){
            py::print("syntax error");
        } else {
            throw;
        }
*/
    }catch(const std::exception& ex){
        printf("%s",ex.what());
        assert(!"unknown exception");
        return -10;
    }
    //}catch(const std::exception& ex){        printf("%s",ex.what());    }
    return 1;
};

PYBIND11_MODULE(astra, module){
    module.doc() = "Native helpers for Astra lib";

    ////////////// simple.test.begin ////////////////
    // CLASSES
    //py::class_<Pet>(module, "Pet")
    //	.def(py::init<const std::string &>())
    //	.def("setName", &Pet::setName)
    //	.def("getName", &Pet::getName);
     add_Pet_Func(
        py::class_<Pet>(module, "Pet")
        .def(py::init<const std::string &>()) // create new Pet object
    );
    // FUNCTIONS
    // expose add function, and add keyword arguments and default arguments
    module.def("add", &add, "A Rastr function which adds two numbers", py::arg("i") = 1, py::arg("j") = 2);
}
#endif //PY_ASTRA


