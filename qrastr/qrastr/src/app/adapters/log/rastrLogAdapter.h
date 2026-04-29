#pragma once
#include "ILogEvents.h"
#include <memory>

class QAstra;

// Адаптер: пробрасывает Qt-сигналы QAstra в ILogEvents.
// QAstra не знает об этом классе.
class RastrLogAdapter : public ILogEvents {
    Q_OBJECT
public:
    explicit RastrLogAdapter(std::shared_ptr<QAstra> qastra,
                             QObject* parent = nullptr);

private:
    std::shared_ptr<QAstra> m_qastra;
};