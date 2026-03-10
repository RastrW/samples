#pragma once

#include <QThread>
#include <QString>
#include <QAtomicInt>

#include <mutex>
#include <condition_variable>
#include <map>
#include <string>
#include <filesystem>

// Forward-объявление интерфейса (из astra/IPlainRastrWrappers.h)
class IPlainRastr;

/// @brief Рабочий поток графического бэкенда.
///
/// Инкапсулирует:
///   • загрузку libastra.so и libSVGgenerator.so через dlopen;
///   • инициализацию Plain-объекта и HTTP-сервера;
///   • цикл обработки асинхронных callback-сообщений.
class GraphicsWorker : public QThread
{
    Q_OBJECT

public:
    // ── Псевдонимы типов функций из libSVGgenerator.so ───────────────────────
    using AsyncCallback_t       = void(*)(int iMSG, const char* params);
    using rastr_t               = IPlainRastr*(*)();
    using InitPlainDLL_t        = void(*)(IPlainRastr*, const char* libpath,
                                          const char* ip, long port,
                                          AsyncCallback_t fn);
    using PutTextLayer_t        = void(*)(long iLayer);
    using UpdateAllContent_t    = void(*)();
    using RemoveGraphNode_t     = void(*)(long inode);
    using MoveOrAddGraphNode_t  = void(*)(long inode, long x, long y);

    // ── Конструктор / деструктор ─────────────────────────────────────────────

    /// @param projectPath  Путь к корневой директории проекта
    ///                     (там лежат libSVGgenerator.so, graph2libs.xml, all/).
    /// @param astraLibPath Полный путь к libastra.so.
    /// @param ip           IP-адрес HTTP-сервера (по умолчанию "127.0.0.0").
    /// @param port         Порт HTTP-сервера (по умолчанию 8081).
    /// @param parent       Родительский QObject.
    explicit GraphicsWorker(
        const std::filesystem::path& projectPath,
        const std::filesystem::path& astraLibPath,
        const QString& ip   = QStringLiteral("127.0.0.0"),
        long            port = 8081,
        QObject*        parent = nullptr);

    ~GraphicsWorker() override;

    // ── Управление ───────────────────────────────────────────────────────────

    /// @brief Запросить мягкую остановку потока.
    ///        Безопасно вызывать из любого потока.
    void requestStop();

    /// @brief URL страницы графики (доступен после сигнала serverReady).
    [[nodiscard]] QString serverUrl() const;

signals:
    // ── Сигналы ──────────────────────────────────────────────────────────────

    /// Сервер успешно поднят и готов принимать подключения.
    void serverReady(const QString& url);

    /// Произошла неустранимая ошибка (поток завершается).
    void errorOccurred(const QString& message);

    /// Информационное сообщение (для отладки / статусной строки).
    void statusMessage(const QString& message);

protected:
    // ── QThread ──────────────────────────────────────────────────────────────
    void run() override;

private:
    // ── Статический callback, который передаётся в InitPlainDLL ──────────────
    /// Пишет в s_pendingCalls и будит главный цикл потока.
    static void staticAsyncCallback(int iMSG, const char* params);

    // ── Обработка одного сообщения из очереди ────────────────────────────────
    void dispatchMessage(int iMSG, const std::string& params);

    // ── Загрузка библиотек и символов ────────────────────────────────────────
    [[nodiscard]] bool loadLibraries();
    void unloadLibraries();

    // ── Параметры ─────────────────────────────────────────────────────────────
    std::filesystem::path   m_projectPath;
    std::filesystem::path   m_astraLibPath;
    QString                 m_ip;
    long                    m_port;

    // ── Дескрипторы библиотек ─────────────────────────────────────────────────
    void* m_hAstra   = nullptr;
    void* m_hSVG     = nullptr;

    // ── Указатели на функции ──────────────────────────────────────────────────
    rastr_t              m_fnPlainFactory    = nullptr;
    InitPlainDLL_t       m_fnInitPlainDLL    = nullptr;
    PutTextLayer_t       m_fnPutTextLayer    = nullptr;
    UpdateAllContent_t   m_fnUpdateAllContent= nullptr;
    RemoveGraphNode_t    m_fnRemoveGraphNode = nullptr;
    MoveOrAddGraphNode_t m_fnMoveOrAddNode   = nullptr;

    // ── Plain-объект ──────────────────────────────────────────────────────────
    IPlainRastr* m_plain = nullptr;

    // ── Флаг остановки ────────────────────────────────────────────────────────
    QAtomicInt m_stopRequested{0};

    // ── Очередь callback-сообщений (разделяется со статическим callback) ──────
    // Используем глобальный (static) экземпляр, чтобы C-callback мог добраться.
    // Защита — ws_mutex / ws_cv.
    static std::mutex              s_mutex;
    static std::condition_variable s_cv;
    static std::map<int, std::string> s_pendingCalls;
    // Указатель на «живой» экземпляр потока (для статического callback).
    static GraphicsWorker*         s_instance;
};
