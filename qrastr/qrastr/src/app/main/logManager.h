#pragma once
#include <QObject>
#include <spdlog/spdlog.h>

namespace ads { class CDockManager; class CDockWidget; }
class MainProtocolWidget;
class GlobalProtocolWidget;
class QAstra;

/**
 * @class LogManager
 * @brief Управляет виджетами логирования (dock-виджеты протоколов).
 *        По аналогии с IGraphManager — создаёт, хранит и регистрирует доки.
 */
class LogManager : public QObject {
    Q_OBJECT
public:
    explicit LogManager(ads::CDockManager* dockManager,
                        QWidget* parent = nullptr);

    /// Создать виджеты (вызывается в конструкторе MainWindow ДО setupLogSinks)
    void createWidgets();

    /// Добавить dock-виджеты в CDockManager (вызывается после restoreState ADS)
    void setupDockWidgets();

    /// Показать / открыть протокол (для пункта меню)
    void openProtocol();

    GlobalProtocolWidget* globalProtocol() const { return m_globalProtocol; }
    MainProtocolWidget*    mainProtocol()   const { return m_mainProtocol; }

    std::shared_ptr<spdlog::sinks::sink> getProtocolLogSink() const {
        return m_qtLogSink;
    }
    void setupLogSinks();
    void setupRastrConnections(std::shared_ptr<QAstra> qastra);
    /// Удалить Qt-синки из spdlog ДО разрушения виджетов протоколов.
    /// Необходимо вызывать до ~FormManager (в MainWindow::closeEvent).
    void teardownLogSinks();
signals:
    /// Испускается при создании каждого dock-виджета —
    /// FormManager подключит его к registerDockWidget
    void dockWidgetCreated(ads::CDockWidget* dw);

private:
    ads::CDockManager*  m_dockManager    = nullptr;
    QWidget*            m_parentWidget   = nullptr;

    GlobalProtocolWidget* m_globalProtocol = nullptr;
    MainProtocolWidget* m_mainProtocol   = nullptr;

    ads::CDockWidget*   m_dockGlobal     = nullptr;
    ads::CDockWidget*   m_dockMain       = nullptr;
    // сохраняем в setupLogSinks
    std::shared_ptr<spdlog::sinks::sink> m_qtLogSink;      // → m_globalProtocol
    std::shared_ptr<spdlog::sinks::sink> m_protocolSink;   // → m_mainProtocol
};