#include "startupLoader.h"

#include "rastrParameters.h"
#include <spdlog/spdlog.h>
#include <QMessageBox>
#include <filesystem>

namespace fs = std::filesystem;

StartupLoader::StartupLoader(std::shared_ptr<QAstra> qastra,
                             QWidget*                parentWidget,
                             QObject*                parent)
    : QObject(parent)
    , m_qastra(std::move(qastra))
    , m_parentWidget(parentWidget)
{
    assert(m_qastra != nullptr);
}

bool StartupLoader::load() {
    auto* params = RastrParameters::get_instance();

#if (QT_VERSION > QT_VERSION_CHECK(5, 16, 0))
    const fs::path templatesDir = params->getDirSHABLON().filesystemCanonicalPath();
#else
    const fs::path templatesDir = params->getDirSHABLON().canonicalPath().toStdString();
#endif

    if (!loadTemplates(templatesDir))
        return false;

    loadFiles(templatesDir);   // некритично — всегда true, ошибки через сигнал
    return true;
}

bool StartupLoader::loadTemplates(const fs::path& templatesDir) {
    const auto& templates = RastrParameters::get_instance()->getStartLoadTemplates();

    for (const auto& templateName : templates) {
        const fs::path fullPath = templatesDir / templateName;

        const IPlainRastrRetCode res =
            m_qastra->Load(eLoadCode::RG_REPL, "", fullPath.string());

        if (res != IPlainRastrRetCode::Ok) {
            spdlog::error("Failed to load template: {}", templateName);
            QMessageBox::critical(
                m_parentWidget,
                QObject::tr("Ошибка"),
                QString(QObject::tr("Не удалось загрузить шаблон:\n%1"))
                    .arg(QString::fromStdString(templateName)));
            return false;   // шаблон критичен
        }
        spdlog::info("Template loaded: {}", templateName);
    }
    return true;
}

bool StartupLoader::loadFiles(const fs::path& templatesDir) {
    const auto& fileList = RastrParameters::get_instance()->getStartLoadFileTemplates();

    for (const auto& [filePath, templateName] : fileList) {
		// Проверяем существование до загрузки
        if (!fs::exists(filePath)) {
            //Нет файла - информируем пользователя и идем дальше.
            spdlog::warn("Startup file not found: {}", filePath);
            const QString msg =
                QString("Файл не найден:\n%1\n\n")
                    .arg(QString::fromStdString(filePath));
            emit loadWarning(msg);
            QMessageBox::warning(m_parentWidget, QObject::tr("Файл не найден"), msg);
            continue;
        }
        try {
            const fs::path fullTemplatePath = templatesDir / templateName;

            const IPlainRastrRetCode res = m_qastra->Load(
                eLoadCode::RG_REPL,
                filePath,
                fullTemplatePath.string());

            if (res == IPlainRastrRetCode::Ok) {
                spdlog::info("Startup file loaded: {}", filePath);
            } else {
                spdlog::error("Failed to load startup file: {} (code {})",
                              filePath, static_cast<int>(res));
                emit loadWarning(
                    QString(QObject::tr("Не удалось загрузить файл:\n%1"))
                        .arg(QString::fromStdString(filePath)));
            }
        } catch (const std::exception& ex) {
            spdlog::error("Exception loading startup file {}: {}", filePath, ex.what());
            QMessageBox::critical(
                m_parentWidget,
                QObject::tr("Ошибка"),
                QString(QObject::tr("Исключение при загрузке файла:\n%1"))
                    .arg(QString::fromStdString(filePath)));
        }
    }
    return true;
}