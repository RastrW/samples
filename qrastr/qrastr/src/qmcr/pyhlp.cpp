#include "pyhlp.h"
#define PY_SSIZE_T_CLEAN

#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG 1
#else
    #include <Python.h>
#endif

#include <regex>
#include <QCoreApplication>
#include <spdlog/spdlog.h>
#include "../app/utilities/pathHelper.h"

namespace PyUtils {

class PyObjRaii
{
public:
    PyObjRaii() = delete;

    explicit PyObjRaii(PyObject* obj) noexcept
        : m_obj(obj) {}

    // Запрет копирования — иначе два деструктора вызовут Py_XDECREF дважды
    PyObjRaii(const PyObjRaii&)            = delete;
    PyObjRaii& operator=(const PyObjRaii&) = delete;

    // Move-семантика
    PyObjRaii(PyObjRaii&& other) noexcept
        : m_obj(std::exchange(other.m_obj, nullptr)) {}

    PyObjRaii& operator=(PyObjRaii&& other) noexcept {
        if (this != &other) {
            Py_XDECREF(m_obj);
            m_obj = std::exchange(other.m_obj, nullptr);
        }
        return *this;
    }

    ~PyObjRaii() {
        if (m_obj) {
            assert(m_obj->ob_refcnt > 0);
            Py_XDECREF(m_obj);
        }
    }

    // Неявное преобразование для передачи в Python C API
    operator PyObject*() const noexcept { return m_obj; }

    bool isNull() const noexcept { return m_obj == nullptr; }

private:
    PyObject* m_obj {nullptr};
};

// Конвертация PyObject* в std::string через PyObject_Repr + utf-8
static std::string pyObjToStr(PyObject* obj)
{
    if (!obj)
        return "__null__";

    PyObjRaii repr(PyObject_Repr(obj));
    if (repr.isNull())
        return "__repr_failed__";

    PyObjRaii encoded(PyUnicode_AsEncodedString(repr, "utf-8", "strict"));
    if (encoded.isNull())
        return "__encode_failed__";

    const char* s = PyBytes_AsString(encoded);
    return s ? s : "__bytes_failed__";
}
/// Возвращает путь к директории Python
std::wstring detectPythonHome()
{
#ifdef _WIN32
    // Windows: ищем через PYTHONHOME → PATH → пусто
    wchar_t buf[32767];
    if (GetEnvironmentVariableW(L"PYTHONHOME", buf, _countof(buf)) > 0) {
        std::wstring home(buf);
        spdlog::info("Python home detected via PYTHONHOME: {}",
                     std::string(home.begin(), home.end()));
        return home;
    }

    wchar_t fullPath[MAX_PATH];
    if (SearchPathW(nullptr, L"python.exe", nullptr, MAX_PATH, fullPath, nullptr) > 0) {
        std::wstring ws(fullPath);
        const auto slash = ws.rfind(L'\\');
        if (slash != std::wstring::npos) {
            std::wstring home = ws.substr(0, slash);
            spdlog::info("Python executable found in PATH: {}",
                         std::string(home.begin(), home.end()));
            return home;
        }
    }

    spdlog::warn("Python home not found via PYTHONHOME or PATH");
    return {};

#else
    // Linux: три контекста
    PathHelper::logContext();

    // Контекст 1: AppImage — Python должен быть в AppDir/usr/lib/
    const QString appImageEnv = QString::fromStdString(
        std::getenv("APPIMAGE") ? std::getenv("APPIMAGE") : "");

    if (!appImageEnv.isEmpty()) {
        const QString appDir = QFileInfo(appImageEnv).absolutePath();
        const QString pythonHome = appDir + "/usr";
        spdlog::info("Python home (AppImage): {}", pythonHome.toStdString());
        return pythonHome.toStdWString();
    }

    // Контекст 2: QtCreator — берём системный Python
    // Контекст 3: SystemInstall — берём системный Python

    const char* h = std::getenv("PYTHONHOME");
    if (h && *h) {
        std::string s(h);
        spdlog::info("Python home detected via PYTHONHOME: {}", s);
        return std::wstring(s.begin(), s.end());
    }

    spdlog::info("Python home not set (using system defaults)");
    return {};
#endif
}

/// Логирует значения важных Python переменных окружения
static void logPythonEnv()
{
    const char* vars[] = { "PYTHONHOME", "PYTHONPATH", "PYTHONEXECUTABLE" };
    for (const char* var : vars) {
        const char* val = std::getenv(var);
        if (val && *val) {
            spdlog::info("Python env {}: {}", var, val);
        }
    }
}
}

#include <pyerrors.h>
#include <frameobject.h>
PyHlp::PyHlp(const IPlainRastr& ipr)
    :m_ipr(ipr){
}

PyHlp::~PyHlp()
{
    if(m_initialized){
        Py_XDECREF(m_rastrObject);
        Py_XDECREF(m_astraModule);
        Py_FinalizeEx();
    }
}

bool PyHlp::initialize()
{
    if (m_initialized)  return true;
    if (Py_IsInitialized()) {
        spdlog::warn("PyHlp::initialize: Python already initialized externally");
        return false;
    }

    spdlog::info("=== PyHlp::initialize() START ===");
    PyUtils::logPythonEnv();

    // ── Шаг 1: Конфигурация Python ──────────────────────────────────────
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    const std::wstring home = PyUtils::detectPythonHome();
    if (!home.empty()) {
        PyStatus st = PyConfig_SetString(&config, &config.home, home.c_str());
        if (PyStatus_Exception(st)) {
            spdlog::warn("PyHlp: не удалось задать home='{}': {}",
                         std::string(home.begin(), home.end()),
                         st.err_msg ? st.err_msg : "?");
        }
    }

    // ── Шаг 2: Инициализация ────────────────────────────────────────────
    PyStatus status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);

    if (PyStatus_Exception(status)) {
        spdlog::error("PyHlp: Py_InitializeFromConfig failed: {}",
                      status.err_msg ? status.err_msg : "неизвестная ошибка");
        return false;
    }

    spdlog::info("Python initialized successfully");

    // ── Шаг 3: Добавляем папку с astra_py в sys.path ──────────────────
    PyObject* sysPath = PySys_GetObject("path"); // borrowed ref
    if (!sysPath) {
        spdlog::error("PyHlp: Failed to get sys.path");
        captureError();
        Py_FinalizeEx();
        return false;
    }

    const QString astraPyDir = PathHelper::getAstraPyDir();
    PathHelper::logPath("astra_py directory", astraPyDir, true);

    if (!QFileInfo::exists(astraPyDir)) {
        spdlog::error("PyHlp: astra_py directory not found: {}",
                      astraPyDir.toStdString());
        Py_FinalizeEx();
        return false;
    }

    PyUtils::PyObjRaii pathItem(
        PyUnicode_FromString(astraPyDir.toStdString().c_str()));

    if (PyList_Append(sysPath, pathItem) != 0) {
        spdlog::error("PyHlp: Failed to append to sys.path");
        captureError();
        Py_FinalizeEx();
        return false;
    }

    spdlog::info("Added to sys.path: {}", astraPyDir.toStdString());

    // ── Шаг 4: Импортируем модуль astra_py ──────────────────────────────
    spdlog::info("Attempting to import 'astra_py' module...");
    m_astraModule = PyImport_ImportModule("astra_py");

    if (!m_astraModule) {
        spdlog::error("PyHlp: Failed to import 'astra_py' module");
        captureError();
        Py_FinalizeEx();
        return false;
    }

    spdlog::info("Module 'astra_py' imported successfully");

    // ── Шаг 5: Создаём объект Rastr(ptr) ────────────────────────────────
    PyUtils::PyObjRaii rastrClass(
        PyObject_GetAttrString(m_astraModule, "Rastr"));

    if (rastrClass.isNull() || !PyCallable_Check(rastrClass)) {
        spdlog::error("PyHlp: 'Rastr' class not found or not callable in astra_py");
        captureError();
        Py_FinalizeEx();
        return false;
    }

    const uintptr_t ptr = reinterpret_cast<uintptr_t>(&m_ipr);
    PyUtils::PyObjRaii pyPtr(PyLong_FromUnsignedLongLong(ptr));
    PyUtils::PyObjRaii pyArg(PyTuple_Pack(1, static_cast<PyObject*>(pyPtr)));

    if (pyArg.isNull()) {
        spdlog::error("PyHlp: Failed to create tuple for Rastr constructor");
        Py_FinalizeEx();
        return false;
    }

    m_rastrObject = PyObject_CallObject(rastrClass, pyArg);

    if (!m_rastrObject || PyErr_Occurred()) {
        spdlog::error("PyHlp: Failed to create Rastr instance");
        captureError();
        Py_XDECREF(m_rastrObject);
        m_rastrObject = nullptr;
        Py_FinalizeEx();
        return false;
    }

    m_initialized = true;
    spdlog::info("=== PyHlp::initialize() SUCCESS ===");
    return true;
}

PyHlp::Result PyHlp::run(const std::string_view macroText)
{
    if (!initialize()){
         spdlog::error("Ошибка инициализации python");
        return Result::Error;
    }

    // borrowed refs — не декрементируем
    PyObject* mainModule = PyImport_AddModule("__main__");
    if (!mainModule) { captureError(); return Result::Error; }

    PyObject* globals = PyModule_GetDict(mainModule);
    if (!globals)     { captureError(); return Result::Error; }

    // Внедряем astra и rastr в глобальное пространство скрипта
    PyMapping_SetItemString(globals, "astra", m_astraModule);
    PyMapping_SetItemString(globals, "rastr", m_rastrObject);

    PyUtils::PyObjRaii compiled(
        Py_CompileString(macroText.data(), k_scriptName, Py_file_input));

    if (compiled.isNull()) {
        captureError();
        PyMapping_DelItemString(globals, "astra");
        PyMapping_DelItemString(globals, "rastr");
        return Result::SyntaxError;
    }

    PyUtils::PyObjRaii locals(PyDict_New());
    PyUtils::PyObjRaii result(PyEval_EvalCode(compiled, globals, locals));

    PyMapping_DelItemString(globals, "astra");
    PyMapping_DelItemString(globals, "rastr");

    if (PyErr_Occurred()) {
        captureError();
        return Result::RuntimeError;
    }
    return Result::Ok;
}

void PyHlp::captureError()
{
    m_errorMessage.clear();
    m_errorLine = -1;
    m_errorOffset = -1;

    PyObject *pType {nullptr}, *pValue {nullptr}, *pTraceback {nullptr};
    PyErr_Fetch(&pType, &pValue, &pTraceback);

    if (!pValue) {
        Py_XDECREF(pType);
        Py_XDECREF(pTraceback);
        return;
    }

    PyErr_NormalizeException(&pType, &pValue, &pTraceback);

    if(pType == PyExc_SyntaxError){
        m_errorMessage = PyUtils::pyObjToStr(pValue);

        // Для SyntaxError номер строки берём напрямую из атрибутов
        PyUtils::PyObjRaii lineNo(PyObject_GetAttrString(pValue, "lineno"));
        if (!lineNo.isNull())
            m_errorLine = PyLong_AsLong(lineNo);

        PyUtils::PyObjRaii offset(PyObject_GetAttrString(pValue, "offset"));
        if (!offset.isNull())
            m_errorOffset = PyLong_AsLong(offset);
    }else{
        // Для RuntimeError разворачиваем traceback через модуль traceback
        PyUtils::PyObjRaii tbModule(PyImport_ImportModule("traceback"));
        if (!tbModule.isNull()) {
            // format_exception → читаемый текст
            PyUtils::PyObjRaii fmtFn(PyObject_GetAttrString(tbModule, "format_exception"));
            if (!fmtFn.isNull() && PyCallable_Check(fmtFn)) {
                PyUtils::PyObjRaii formatted(
                    PyObject_CallFunctionObjArgs(fmtFn,
                                                 pType, pValue, pTraceback ? pTraceback : Py_None, nullptr));
                if (!formatted.isNull()) {
                    std::string raw = PyUtils::pyObjToStr(formatted);
                    // Убираем экранированные \n из repr
                    raw = std::regex_replace(raw, std::regex(R"(\\n)"), "\n");
                    m_errorMessage = std::move(raw);
                }
            }
            // extract_tb → ищем номер строки в нашем скрипте
            PyObject* tbDict    = PyModule_GetDict(tbModule); // borrowed
            PyObject* extractTb = PyDict_GetItemString(tbDict, "extract_tb"); // borrowed
            if (extractTb && pTraceback) {
                Py_INCREF(pTraceback);
                PyUtils::PyObjRaii args(PyTuple_Pack(1, pTraceback));
                PyUtils::PyObjRaii frames(PyObject_CallObject(extractTb, args));
                if (!frames.isNull()) {
                    const Py_ssize_t n = PySequence_Size(frames);
                    for (Py_ssize_t i = 0; i < n; ++i) {
                        PyUtils::PyObjRaii frame(PySequence_GetItem(frames, i));
                        PyUtils::PyObjRaii fname(PySequence_GetItem(frame, 0));
                        PyUtils::PyObjRaii lineno(PySequence_GetItem(frame, 1));

                        std::string name = PyUtils::pyObjToStr(fname);
                        // имя приходит в кавычках вида 'rastr-py-embedded'
                        if (name.size() > 2)
                            name = name.substr(1, name.size() - 2);
                        if (name == k_scriptName)
                            m_errorLine = PyLong_AsLong(lineno);
                    }
                }
            }
        }

        if (m_errorMessage.empty())
            m_errorMessage = PyUtils::pyObjToStr(pValue);
    }

    //Освобождаем refs, очищаем индикатор, после Fetch ошибка уже снята
    Py_XDECREF(pType);
    Py_XDECREF(pValue);
    Py_XDECREF(pTraceback);
}

std::string PyHlp::getErrorMessage() const noexcept{
    return m_errorMessage;
}

long PyHlp::getErrorLine() const noexcept{
    return m_errorLine;
}

long PyHlp::getErrorOffset() const noexcept{
    return m_errorOffset;
}
