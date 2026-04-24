#pragma once
#include <astra/IPlainRastr.h>

class IFileOperations {
public:
    virtual ~IFileOperations() = default;

    virtual IPlainRastrRetCode Load(eLoadCode        loadCode,
                                    std::string_view filePath,
                                    std::string_view templatePath) = 0;

    virtual void Save(std::string_view filePath,
                      std::string_view templatePath) = 0;
};