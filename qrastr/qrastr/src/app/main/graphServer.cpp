#include "graphServer.h"
#include <QThread>
#include <QCoreApplication>
#include "graphWorker.h"
#include "astra/IPlainRastr.h"

GraphServer* GraphServer::s_instance = nullptr;

GraphServer::GraphServer(IPlainRastr* rastr, QObject* parent)
    : QObject(parent),
    m_rastr(rastr){

    const QString libDir = QCoreApplication::applicationDirPath() + "/";
    m_lib.setFileName(libDir + "SVGgenerator");
}

GraphServer::~GraphServer() {
    stop();
    qInfo() << "GraphServer was deleted";
}

bool GraphServer::start() {
    if (isRunning()) return true;

    // Создаём поток и рабочий объект
    m_thread = new QThread(this);
    m_worker = new GraphWorker(m_rastr, &m_lib);   // без parent — будет перемещён
    m_worker->moveToThread(m_thread);

    // Жизненный цикл: поток стартовал → запускаем инициализацию
    connect(m_thread, &QThread::started,  m_worker, &GraphWorker::slot_process);

    // Рабочий завершил (ошибка или нормально) → останавливаем поток
    connect(m_worker, &GraphWorker::sig_finished, m_thread, &QThread::quit);

    // Поток завершился → чистим ресурсы и сигналим наружу
    connect(m_thread, &QThread::finished, this, [this]() {
        s_instance = nullptr;
        qInfo() << "[sig_stopped] delivered in thread:" << QThread::currentThreadId();
        // Чистим синхронно в stop(), здесь только сигнал
        emit sig_stopped();
    });

    // Готовность сервера → пробрасываем сигнал наружу
    connect(m_worker, &GraphWorker::sig_ready, this, [this]() {
        s_instance = this;
        qInfo() << "[sig_ready] delivered in thread:" << QThread::currentThreadId();
        emit sig_ready();
    });

    m_thread->start();
    return true;
}

void GraphServer::stop() {
    if (!m_thread || !m_thread->isRunning()) return;

    if (m_worker)
        m_worker->stopFromOutside();

    // Ждём штатного завершения
    if (!m_thread->wait(5000)) {
        qWarning() << "GraphServer: thread did not stop in 5 s — terminating";
        m_thread->terminate();
        m_thread->wait(2000); // ещё одна попытка после terminate
    }

    // Только после завершения потока можно безопасно выгружать DLL и удалять объекты
    if (m_lib.isLoaded()) {
        m_lib.unload();
        qInfo() << "GraphServer: library unloaded";
    }
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    delete m_thread;
    m_thread = nullptr;
}

 bool GraphServer::isRunning() const {
     return m_thread && m_thread->isRunning();
 }

void GraphServer::staticCallback(int iMSG, const char* params) {
    if (!s_instance || !s_instance->m_worker) return;

    // QueuedConnection: вызов встаёт в очередь потока m_worker — никаких mutex не нужно
    QMetaObject::invokeMethod(
        s_instance->m_worker,
        "slot_handleCallback",
        Qt::QueuedConnection,
        Q_ARG(int,     iMSG),
        Q_ARG(QString, QString::fromUtf8(params ? params : ""))
    );
}
