#include "logManager.h"
#include "protocollogwidget.h"
#include "protocolwidget.h"
#include <DockManager.h>
#include <DockWidget.h>
#include "qastra.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/qt_sinks.h>

LogManager::LogManager(ads::CDockManager* dockManager,
                       QWidget* parent)
    : QObject(parent)
    , m_dockManager(dockManager)
    , m_parentWidget(parent)
{}

void LogManager::setupLogSinks()
{
    auto logger = spdlog::default_logger();

    auto qtSink = std::make_shared<spdlog::sinks::qt_sink_mt>(
        m_globalProtocol, "onAppendText");
    logger->sinks().push_back(qtSink);
    m_qtLogSink = qtSink;

    auto protocolSink = std::make_shared<spdlog::sinks::qt_sink_mt>(
        m_mainProtocol, "onAppendProtocol");
    logger->sinks().push_back(protocolSink);
    m_protocolSink = protocolSink;
}

void LogManager::setupRastrConnections(std::shared_ptr<QAstra> qastra) {
    // Второй поток данных — структурированные _log_data от Rastr
    connect(qastra.get(), &QAstra::onRastrLog,
            m_mainProtocol,   &ProtocolWidget::onRastrLog);
    connect(qastra.get(), &QAstra::onRastrLog,
            m_globalProtocol, &ProtocolLogWidget::onRastrLog);
}

void LogManager::createWidgets() {
    m_globalProtocol = new ProtocolLogWidget(m_parentWidget);
    m_mainProtocol = new ProtocolWidget(m_parentWidget);
    m_mainProtocol->setIgnoreAppendProtocol(true);
}

void LogManager::setupDockWidgets() {
    // --- Глобальный протокол ---
    m_dockGlobal = new ads::CDockWidget("globalProtocol", m_parentWidget);
    m_dockGlobal->setWidget(m_globalProtocol);
    m_dockGlobal->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    // Скрываем dock, но не удаляем виджет внутри
    connect(m_dockGlobal, &ads::CDockWidget::closeRequested,
            m_dockGlobal, [this]() { m_dockGlobal->toggleView(false); });
    m_dockManager->addDockWidgetTab(ads::BottomDockWidgetArea, m_dockGlobal);
    emit dockWidgetCreated(m_dockGlobal);
    // --- Протокол Astra ---
    m_dockMain = new ads::CDockWidget("Протокол Astra", m_parentWidget);
    m_dockMain->setWidget(m_mainProtocol);
    m_dockMain->setFeature(ads::CDockWidget::CustomCloseHandling, true);
    connect(m_dockMain, &ads::CDockWidget::closeRequested,
            m_dockMain, [this]() { m_dockMain->toggleView(false); });
    m_dockManager->addDockWidgetTab(ads::BottomDockWidgetArea, m_dockMain);
    emit dockWidgetCreated(m_dockMain);
}

void LogManager::openProtocol() {
    if (!m_dockGlobal) return;

    if (m_dockGlobal->isClosed()) {
        m_dockGlobal->toggleView(true);
    }

    if (m_dockMain->isClosed()) {
        m_dockMain->toggleView(true);
    }
}

void LogManager::teardownLogSinks()
{
    auto logger = spdlog::default_logger();
    auto& sinks = logger->sinks();

    // Удаляем оба Qt-синка за один проход
    sinks.erase(
        std::remove_if(sinks.begin(), sinks.end(),
                       [this](const std::shared_ptr<spdlog::sinks::sink>& s) {
                           return s == m_qtLogSink || s == m_protocolSink;
                       }),
        sinks.end()
        );

    m_qtLogSink.reset();
    m_protocolSink.reset();
}