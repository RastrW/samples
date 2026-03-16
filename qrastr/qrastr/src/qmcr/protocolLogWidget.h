#pragma once
#include <QWidget>
#include "../app/astra/qastra_events_data.h"
#include "qmcr_api.h"

class SciLogViewer;

/// @class Виджет отображения лога расчёта.
class QMCR_API ProtocolLogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProtocolLogWidget(QWidget* parent = nullptr);
    ~ProtocolLogWidget() override = default;

    /// Очистить содержимое лога
    void clear();

public slots:
    void onRastrLog(const _log_data& log_data);
    void onRastrPrint(const std::string& str_msg);
    void onQStringAppendProtocol(const QString& qstr);

private:
    SciLogViewer* m_viewer { nullptr };
    long    n_stage_max_id_{ 0 };

    /// HTML-экранирование (перенесено из McrWnd без изменений)
    static void encodeHtml(std::string& data);
    static void encodeHtml(std::string& data_out, const QString& qstr_in);
};
