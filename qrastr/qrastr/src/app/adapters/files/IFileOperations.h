#pragma once
#include <astra/IPlainRastr.h>

class IFileOperations {
public:
    virtual ~IFileOperations() = default;

    /// @return true при успехе
    [[nodiscard]]
    virtual bool Load(eLoadCode        loadCode,
                      std::string_view filePath,
                      std::string_view templatePath) noexcept = 0;

    /// @return true при успехе
    [[nodiscard]]
    virtual bool Save(std::string_view filePath,
                      std::string_view templatePath) noexcept = 0;

    /// @brief Детали последней ошибки (пусто, если операция прошла успешно).
    [[nodiscard]]
    virtual std::string lastError() const noexcept = 0;
};