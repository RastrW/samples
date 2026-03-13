#pragma once
#include <QObject>
#include <QLibrary>

class IPlainRastr;
class GraphWorker;

//  GraphServer — управляет жизненным циклом потока
class GraphServer : public QObject {
    Q_OBJECT
public:
    explicit GraphServer(IPlainRastr* rastr, QObject* parent = nullptr);
    ~GraphServer() override;

    bool start();
    void stop();
    bool isRunning() const;

    /// Статический мост: вызывается из dll (любой поток) → безопасно встаёт в очередь Qt
    static void staticCallback(int iMSG, const char* params);
signals:
    void sig_ready();
    void sig_stopped();

private:
    IPlainRastr*   m_rastr  = nullptr;
    QLibrary       m_lib;
    QThread*       m_thread = nullptr;
    GraphWorker*   m_worker = nullptr;

    //Статическое поле гарантирует, что возможна работа
    //только одного серверы одновременно (один порт 8081)
    static GraphServer* s_instance;
};
