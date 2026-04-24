#include "rastrFileAdapter.h"
#include "QAstra.h"

RastrFileAdapter::RastrFileAdapter(std::shared_ptr<QAstra> qastra)
    : m_qastra(std::move(qastra))
{
    assert(m_qastra != nullptr);
}

IPlainRastrRetCode RastrFileAdapter::Load(eLoadCode        loadCode,
                                          std::string_view filePath,
                                          std::string_view templatePath)
{
    return m_qastra->Load(loadCode, filePath, templatePath);
}

void RastrFileAdapter::Save(std::string_view filePath,
                            std::string_view templatePath)
{
    m_qastra->Save(filePath, templatePath);
}