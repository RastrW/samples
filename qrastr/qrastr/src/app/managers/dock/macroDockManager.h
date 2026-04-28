#pragma once
#include <QObject>
#include <QPointer>
#include <memory>

namespace ads { class CDockManager; class CDockWidget; }
class McrWnd;
class PyHlp;
class ILogEvents;
class QWidget;

/**
 * @class MacroDockManager
 * @brief Управляет единственным dock-виджетом редактора макросов.
 *  - создаёт dock один раз при первом вызове openWindow()
 *  - при повторных вызовах только поднимает вкладку
 *  - испускает windowOpened() → FormManager::registerDockWidget()
 *  - перехватывает closeRequested для диалога сохранения
 */
class MacroDockManager : public QObject {
    Q_OBJECT
public:
    explicit MacroDockManager(
        ads::CDockManager*      dockManager,
        std::shared_ptr<PyHlp>  pyHlp,
        std::shared_ptr<ILogEvents> logEvents,
        QWidget*                parent = nullptr);

    /// Открыть или поднять окно макросов.
    void openWindow();

    /// true — dock существует и не скрыт.
    bool isOpen() const;

    /// objectName dock-виджета (используется в openWidgetByName).
    static constexpr const char* kDockObjectName = "Макро редактор";

signals:
    /// Подключается в FormManager к registerDockWidget().
    void windowOpened(ads::CDockWidget* dw);

private:
    ads::CDockManager*      m_dockManager;
    std::shared_ptr<PyHlp>  m_pyHlp;
    std::shared_ptr<ILogEvents> m_logEvents;
    QWidget*                m_parentWidget;

    QPointer<ads::CDockWidget> m_dockWidget;
    McrWnd*                    m_mcrWnd {nullptr};
};