#include "graphWebManager.h"
#include "graphServer.h"
#include <QWebEngineView>
#include <spdlog/spdlog.h>
#include <DockWidget.h>
#include <DockManager.h>
#include <QTimer>
#include <QTcpSocket>

GraphWebManager::GraphWebManager(ads::CDockManager* dockManager,
                                 QWidget*           parentWidget,
                                 IPlainRastr*       rastr,
                                 QObject*           parent)
    : IGraphManager(dockManager, parentWidget, parent)
    , m_rastr(rastr)
{}

void GraphWebManager::openWindow()
{
    // Пересоздаём сервер только если его нет совсем.
    // После stop() объект жив (parent=this), но m_thread == nullptr,
    // поэтому isRunning()==false и start() корректно запустит его снова.
    if (!m_graphServer) {
        m_graphServer = new GraphServer(m_rastr, m_parentWidget);
        // Попадаем сюда только если остановили через закрытие дока
        connect(m_graphServer, &GraphServer::sig_stopped, this, [this]() {
            spdlog::info("GraphWebManager: GraphServer stopped");
            delete m_graphServer;
            m_graphServer = nullptr;
        });
    }

    auto* dw      = new ads::CDockWidget(tr("Графика Web"), m_parentWidget);
    auto* webView = new QWebEngineView(dw);
    dw->setWidget(webView);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

    m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
    m_windowCount++;

    // Загружаем страницу — сразу или после старта сервера
    auto loadPage = [webView]() {
        webView->load(QUrl("http://127.0.0.1:8081/grf.html"));
    };

    if (m_graphServer->isRunning()) {
        loadPage();
    } else {
        //гарантируем, что соединение само отключится после первого срабатывания,
        // сколько бы раз ни открывался dok до готовности сервера.
        auto* conn = new QMetaObject::Connection();
        *conn = connect(m_graphServer, &GraphServer::sig_ready,
                        webView, [loadPage, conn]() {
                            loadPage();
                            QObject::disconnect(*conn);
                            delete conn;
                        },
                        Qt::QueuedConnection);
    }


    // Диагностические подключения
    connect(webView, &QWebEngineView::loadStarted,
            []{ spdlog::info("[webView] loadStarted"); });
    connect(webView, &QWebEngineView::loadProgress,
            [](int p){ spdlog::info("[webView] loadProgress: {}", p); });
    connect(webView, &QWebEngineView::loadFinished,
            [](bool ok){ spdlog::info("[webView] loadFinished, ok= {}", ok); });
    connect(webView->page(), &QWebEnginePage::urlChanged,
            [](const QUrl& url){spdlog::info("[webView] urlChanged: {}", url.toString().toStdString());});

    // Останавливаем сервер при закрытии последнего дока
    connect(dw, &ads::CDockWidget::closeRequested, this, [this]() {
        if (--m_windowCount <= 0) {
            m_windowCount = 0;
            if (m_graphServer)
                m_graphServer->stop();
        }
        emit windowClosed();
        if (m_windowCount == 0)
            emit allWindowsClosed();
    });

    if (!m_graphServer->isRunning())
        m_graphServer->start();

    spdlog::info("GraphWebManager: открыто окно #{}", m_windowCount);
    emit windowOpened(dw);
}

void GraphWebManager::closeAll()
{
	// Останавливаем синхронно ДО того, как Qt начнёт рушить объекты
    if (!m_graphServer) return;
    // Отключаем сигнал, чтобы не сработал deleteLater из слота
    disconnect(m_graphServer, &GraphServer::sig_stopped, this, nullptr);
    m_graphServer->stop(); // теперь гарантированно завершает поток
    delete m_graphServer;
    m_graphServer = nullptr;
    spdlog::info("GraphWebManager: сервер остановлен синхронно");
}