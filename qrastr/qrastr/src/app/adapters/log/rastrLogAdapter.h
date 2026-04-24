#pragma once
#include "ILogSource.h"
#include <memory>

class QAstra;

class RastrLogAdapter : public ILogSource {
public:
    explicit RastrLogAdapter(std::shared_ptr<QAstra> qastra);
};