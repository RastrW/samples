#pragma once
#include <QObject>

struct _log_data;
class _hint_data;

class ILogEvents : public QObject {
    Q_OBJECT
public:
    explicit ILogEvents(QObject* parent = nullptr)
        : QObject(parent) {}
    virtual ~ILogEvents() = default;

signals:
    void sig_rastrLog (const _log_data&);
    void sig_rastrHint(const _hint_data&);
};