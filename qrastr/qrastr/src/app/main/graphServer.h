#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>
#include <QObject>
#include "astra/IPlainRastr.h"

/**
 * @class Управляет жизненным циклом графического сервера (libSVGgenerator).
 * Поток запускается при start() и корректно останавливается при stop().
 */
class GraphServer : public QObject {
    Q_OBJECT
public:
    explicit GraphServer(IPlainRastr* rastr, QObject* parent = nullptr);
    ~GraphServer() override;

    /// Запустить сервер (создаёт поток, загружает dll, поднимает HTTP)
    bool start();
    /// Остановить сервер (будит поток, дожидается завершения)
    void stop();
    bool isRunning() const { return m_running.load(); }
signals:
    /// Сервер готов принимать соединения
    void sig_ready();
    /// Сервер завершил работу
    void sig_stopped();
private:
    void threadFunc();                        // Тело потока
    void dispatchCallback(int iMSG,
                          const std::string& params);

    // --- dll-функции ---
    using InitPlainDLL_t     = void(*)(IPlainRastr*, const char*, const char*, long,
                                    void(*)(int, const char*));
    using PutTextLayer_t     = void(*)(long);
    using UpdateAllContent_t = void(*)();
    using RemoveGraphNode_t  = void(*)(long);
    using MoveOrAddGraphNode_t = void(*)(long, long, long);

    IPlainRastr*           m_rastr   = nullptr;
    void*                  m_dllHandle = nullptr;

    InitPlainDLL_t         m_fnInit          = nullptr;
    PutTextLayer_t         m_fnPutTextLayer  = nullptr;
    UpdateAllContent_t     m_fnUpdateAll     = nullptr;
    RemoveGraphNode_t      m_fnRemoveNode    = nullptr;
    MoveOrAddGraphNode_t   m_fnMoveOrAdd     = nullptr;

    std::thread            m_thread;
    std::atomic<bool>      m_running  {false};
    std::atomic<bool>      m_stopFlag {false};

    // Очередь колбэков
    std::mutex                    m_mutex;
    std::condition_variable       m_cv;
    std::map<int, std::string>    m_calls;

    // Статический мост: GraphServer* -> AsyncCallback
    static GraphServer*           s_instance; // Одно статическое поле => сервер только один
    static void staticCallback(int iMSG, const char* params);
};
