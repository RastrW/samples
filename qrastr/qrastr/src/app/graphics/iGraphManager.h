#pragma once
#include <QObject>

namespace ads { class CDockWidget; class CDockManager; }

/// @brief Общий интерфейс менеджеров окон графики.
class IGraphManager : public QObject
{
    Q_OBJECT
public:
    explicit IGraphManager(ads::CDockManager* dockManager,
                           QWidget*           parentWidget,
                           QObject*           parent = nullptr)
        : QObject(parent)
        , m_dockManager(dockManager)
        , m_parentWidget(parentWidget)
    {}

    ~IGraphManager() override = default;
	/// открыть новое окно-вкладку в CDockManager.
    virtual void openWindow() = 0;
    /// Синхронная остановка. Вызывать перед уничтожением приложения.
    virtual void closeAll() = 0;
    virtual int openWindowCount() const = 0;
signals:
    /// эмитируется после создания dock-виджета,
    void windowOpened(ads::CDockWidget* dw);
    void windowClosed();
    void allWindowsClosed();

protected:
    ads::CDockManager* m_dockManager  = nullptr;
    QWidget*           m_parentWidget = nullptr;
};