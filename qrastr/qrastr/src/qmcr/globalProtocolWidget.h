#pragma once
#include <QWidget>
#include "../app/plugin_wrappers/qastra_events_data.h"
#include "qmcr_api.h"

class SciLogViewer;

/// @class Виджет отображения лога расчёта.
class QMCR_API GlobalProtocolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GlobalProtocolWidget(QWidget* parent = nullptr);
    ~GlobalProtocolWidget() override = default;
    /// Очистить содержимое лога
    void clear();

public slots:
    void onRastrLog  (const _log_data&   logData);
    void onRastrPrint(const std::string& msg);
    void onAppendText(const QString&     text);

private:
    SciLogViewer* m_viewer { nullptr };
    long          m_stageMaxId { 0 };

    /// Возвращает строку из n символов '\t'
    static std::string makeIndent(int n);
};
