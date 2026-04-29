#pragma once
#include "IFileOperations.h"
#include <memory>

class QAstra;

class RastrFileAdapter : public IFileOperations {
public:
    explicit RastrFileAdapter(std::shared_ptr<QAstra> qastra);

    [[nodiscard]]
    bool Load(eLoadCode        loadCode,
              std::string_view filePath,
              std::string_view templatePath) noexcept override;

    [[nodiscard]]
    bool Save(std::string_view filePath,
                  std::string_view templatePath) noexcept override;

    std::string lastError() const noexcept override { return m_lastError; }
private:
    std::shared_ptr<QAstra> m_qastra;
    mutable std::string     m_lastError;
};