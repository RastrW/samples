#include "hostWidget.h"
#include <spdlog/spdlog.h>
#include <QResizeEvent>


HostWidget::HostWidget(QWidget* parent)
    : QWidget(parent)
{
    //setAttribute(Qt::WA_NativeWindow);      //с WA_NativeWindow WM_PAINT не приходит при первом открытии
    setAttribute(Qt::WA_PaintOnScreen);             //указывает Qt рисовать напрямую на экране (в обход графического конвейера Qt)
//  setAttribute(Qt::WA_UpdatesDisabled);           //программный «выключатель» отрисовки виджета. Когда он установлен, любые запросы на перерисовку (update(), repaint()) игнорируются, а события paintEvent не доставляются
//  setUpdatesEnabled(false);
    setAttribute(Qt::WA_OpaquePaintEvent);          //виджет самостоятельно закрашивает всю свою область непрозрачным цветом
    setAttribute(Qt::WA_NoSystemBackground);        //виджет не должен запрашивать у операционной системы отрисовку стандартного фона окна перед вызовом события paintEvent
    //setAttribute(Qt::WA_PendingResizeEvent)
    setAttribute(Qt::WA_Resized);


    setMinimumSize(100, 100);
}

HostWidget::~HostWidget()
{
    onClose();
}

bool HostWidget::init()
{
    void* parentHandle = reinterpret_cast<void*>(winId());
    spdlog::info("HostWidget::init: winId = 0x{:016X}",
                 reinterpret_cast<uintptr_t>(parentHandle));

#if defined(Q_OS_WIN)
    // WS_CLIPCHILDREN — не рисовать поверх дочерних HWND при WM_PAINT родителя.
    // WS_CLIPSIBLINGS — не рисовать поверх окон-соседей с тем же родителем.
    {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
        ::SetWindowLongPtrW(hwnd, GWL_STYLE,
                            style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
//        ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
//                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
#endif

    bool testWindow = false;
    if (testWindow) {
#if defined(Q_OS_WIN)
        if (!m_testChild.create(parentHandle, 0, 0, width(), height())) {
            spdlog::error("HostWidget::init: SelfDrawingChild::create завершился с ошибкой");
            return false;
        }
#endif
    }
    if (!testWindow) {
        if (!m_elGraph.init(parentHandle)) {
            spdlog::error("HostWidget::init: ElGraphService::init завершился с ошибкой");
            return false;
        }
    }

    return true;
}

void HostWidget::onClose()
{
#if defined(Q_OS_WIN)
    m_testChild.destroy();
#endif
    m_elGraph.shutdown();
}

void HostWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
#if defined(Q_OS_WIN)
    m_testChild.resize(0, 0, event->size().width(), event->size().height());
#endif
}
