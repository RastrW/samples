#pragma once
#include <QWidget>
#include "elGraphService.h"
#include "test/SelfDrawingChild.h"

/**
 * @class Qt-виджет, напрямую встраивающий нативное дочернее окно — без SDL.
 *
 * Используется вместо SDLHostWidget для диагностики: позволяет проверить,
 * корректно ли дочернее окно встраивается без прослойки SDL.
 */
class HostWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HostWidget(QWidget* parent = nullptr);
    ~HostWidget() override;

    bool init();
    void onClose();

    ElGraphService&       elGraph()       { return m_elGraph; }
    const ElGraphService& elGraph() const { return m_elGraph; }

    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent*) override {}

private:
    ElGraphService   m_elGraph;
    SelfDrawingChild m_testChild;
};