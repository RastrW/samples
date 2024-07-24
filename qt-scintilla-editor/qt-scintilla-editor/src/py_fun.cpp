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

long EmbPyRunMacro( ){
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    std::shared_ptr<Pet> pet = std::make_shared<Pet>("TestttPet");
    auto pets_mod = py::module::import("PetsTst");
    py::globals()["pet"] = py::cast(pet);
    py::exec("pet.setName('testPet')");
    py::exec("pet.bark()");
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


