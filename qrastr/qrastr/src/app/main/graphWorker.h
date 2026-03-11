#pragma once
#include <QObject>

class IPlainRastr;
class QLibrary;

//  GraphWorker — живёт в фоновом потоке
class GraphWorker : public QObject {
    Q_OBJECT
public:
    explicit GraphWorker(IPlainRastr* rastr, QLibrary* lib);
    virtual ~GraphWorker();

public slots:
    /// Вызывается QThread::started — инициализация и запуск websocket
    void slot_process();
    /// Принимает колбэки из staticCallback через QueuedConnection — потокобезопасно
    void slot_handleCallback(int iMSG, const QString& params);
signals:
    void sig_ready();
    void sig_finished();
private:
    // --- типы dll-функций ---
    using InitPlainDLL_t       = void(*)(IPlainRastr*, const char*, const char*,
                                         long, void(*)(int, const char*));
    using PutTextLayer_t       = void(*)(long);
    using UpdateAllContent_t   = void(*)();
    using RemoveGraphNode_t    = void(*)(long);
    using MoveOrAddGraphNode_t = void(*)(long, long, long);

    void loadSymbols();
    void dispatchCallback(int iMSG, const std::string& params);
	
    IPlainRastr*           m_rastr          = nullptr;
    QLibrary*              m_lib            = nullptr;
    InitPlainDLL_t         m_fnInit         = nullptr;
    PutTextLayer_t         m_fnPutTextLayer = nullptr;
    UpdateAllContent_t     m_fnUpdateAll    = nullptr;
    RemoveGraphNode_t      m_fnRemoveNode   = nullptr;
    MoveOrAddGraphNode_t   m_fnMoveOrAdd    = nullptr;
};
