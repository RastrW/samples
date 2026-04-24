#include "rastrLogAdapter.h"
#include "QAstra.h"

RastrLogAdapter::RastrLogAdapter(std::shared_ptr<QAstra> qastra,
                                 QObject* parent)
    : ILogEvents(parent)
    , m_qastra(std::move(qastra))
{
    assert(m_qastra != nullptr);

    // Пробрасываем сигналы QAstra в наши сигналы ILogEvents.
    // Всё соединение — в одном месте, QAstra не трогаем.
    connect(m_qastra.get(), &QAstra::onRastrLog,
            this,           &ILogEvents::sig_rastrLog);
    connect(m_qastra.get(), &QAstra::onRastrHint,
            this,           &ILogEvents::sig_rastrHint);
}