#pragma once
#include "IFileOperations.h"
#include <memory>

class QAstra;

class RastrFileAdapter : public IFileOperations {
public:
    explicit RastrFileAdapter(std::shared_ptr<QAstra> qastra);

    IPlainRastrRetCode Load(eLoadCode        loadCode,
                            std::string_view filePath,
                            std::string_view templatePath) override;

    void Save(std::string_view filePath,
              std::string_view templatePath) override;

private:
    std::shared_ptr<QAstra> m_qastra;
};