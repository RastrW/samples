#pragma once
#include "iGraphManager.h"

class GraphServer;
class IPlainRastr;

/**
 * @brief Управляет жизненным циклом Web-графики.
 *
 * Ответственности:
 *  - Создание/остановка GraphServer (один экземпляр на менеджер).
 *  - Создание CDockWidget + QWebEngineView при каждом openWindow().
 *  - Счётчик окон → остановка сервера при закрытии последнего.
 *  - Эмиссия windowOpened(dw) для регистрации в FormManager.
 */
class GraphWebManager : public IGraphManager
{
    Q_OBJECT
public:
    explicit GraphWebManager(ads::CDockManager* dockManager,
                             QWidget*           parentWidget,
                             IPlainRastr*       rastr,
                             QObject*           parent = nullptr);
    ~GraphWebManager() override = default;

    void openWindow() override;
    /// @brief Синхронная остановка сервера. Вызывать перед закрытием приложения.
    void closeAll()   override;
    int openWindowCount() const override { return m_windowCount; }
private:
    IPlainRastr* m_rastr       = nullptr;
    GraphServer* m_graphServer = nullptr;
    int          m_windowCount = 0;
};