#include "rastrFileAdapter.h"
#include "qastra.h"

RastrFileAdapter::RastrFileAdapter(std::shared_ptr<QAstra> qastra)
    : m_qastra(std::move(qastra))
{
    assert(m_qastra != nullptr);
}

bool RastrFileAdapter::Load(eLoadCode loadCode,
                          std::string_view filePath,
                          std::string_view templatePath) noexcept
{
    try {
        const IPlainRastrRetCode res =
            m_qastra->Load(loadCode, filePath, templatePath);

        if (res != IPlainRastrRetCode::Ok) {
            m_lastError = fmt::format(
                "Load failed (code {}): file='{}' tmpl='{}'",
                static_cast<int>(res), filePath, templatePath);
            spdlog::error("{}", m_lastError);
            return false;
        }

        m_lastError.clear();
        return true;

    } catch (const std::exception& ex) {
        m_lastError = fmt::format("Load exception: {}", ex.what());
        spdlog::error("{}", m_lastError);
        return false;
    } catch (...) {
        m_lastError = "Load: unknown exception";
        spdlog::error("{}", m_lastError);
        return false;
    }
}

bool RastrFileAdapter::Save(std::string_view filePath,
                            std::string_view templatePath) noexcept
{
    try {
        m_qastra->Save(filePath, templatePath);

        m_lastError.clear();
        return true;
    } catch (const std::exception& ex) {
        m_lastError = fmt::format("Save exception: {}", ex.what());
        spdlog::error("{}", m_lastError);
        return false;
    } catch (...) {
        m_lastError = "Save: unknown exception";
        spdlog::error("{}", m_lastError);
        return false;
    }
}
