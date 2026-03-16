#pragma once

#include "sciHlpBase.h"

/// Лог-вьювер: read-only, подсветка XML.
class SciLogViewer : public SciHlpBase
{
    Q_OBJECT
public:
    explicit SciLogViewer(QWidget* parent);
    ~SciLogViewer() override = default;

private:
    void setupXmlLexer();
};
