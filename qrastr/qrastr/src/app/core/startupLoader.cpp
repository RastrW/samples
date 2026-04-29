#include "startupLoader.h"

#include "rastrParameters.h"
#include <spdlog/spdlog.h>
#include <QMessageBox>
#include "files/IFileOperations.h"

StartupLoader::StartupLoader(std::shared_ptr<IFileOperations> fileOps,
                             QWidget*                parentWidget,
                             QObject*                parent)
    : QObject(parent)
    , m_fileOps(std::move(fileOps))
    , m_parentWidget(parentWidget)
{
    assert(m_fileOps != nullptr);
}

bool StartupLoader::load() {
    const QDir templatesDir =
        RastrParameters::get_instance()->getDirSHABLON();

    if (!loadTemplates(templatesDir))
        return false;

    loadFiles(templatesDir); // некритично — всегда true, ошибки через сигнал
    return true;
}

bool StartupLoader::loadTemplates(const QDir& templatesDir) {
    for (const auto& templateName
         : RastrParameters::get_instance()->getStartLoadTemplates())
    {
        const QString fullPath =
            templatesDir.filePath(QString::fromStdString(templateName));

        if (!m_fileOps->Load(eLoadCode::RG_REPL, "", fullPath.toStdString())) {
            const QString detail = QString::fromStdString(m_fileOps->lastError());
            spdlog::error("Failed to load template '{}'", templateName);
            QMessageBox::critical(
                m_parentWidget, QObject::tr("Ошибка"),
                QObject::tr("Не удалось загрузить шаблон:\n%1\n\n%2")
                    .arg(QString::fromStdString(templateName), detail));
            return false; // шаблон критичен
        }
        spdlog::info("Template loaded: {}", templateName);
    }
    return true;
}

void StartupLoader::loadFiles(const QDir& templatesDir) {
    for (const auto& [filePath, templateName]
         : RastrParameters::get_instance()->getStartLoadFileTemplates())
    {
        const QString qFilePath = QString::fromStdString(filePath);
        // Проверяем существование до загрузки
        if (!QFileInfo::exists(qFilePath)) {
            //Нет файла - информируем пользователя и идем дальше.
            const QString msg = QObject::tr("Файл не найден:\n%1").arg(qFilePath);
            spdlog::warn("Startup file not found: {}", filePath);
            emit sig_loadWarning(msg);
            QMessageBox::warning(m_parentWidget, QObject::tr("Файл не найден"), msg);
            continue;
        }

        const QString fullTemplatePath =
            templatesDir.filePath(QString::fromStdString(templateName));

        if (!m_fileOps->Load(eLoadCode::RG_REPL,
                             filePath,
                             fullTemplatePath.toStdString()))
        {
            const QString detail = QString::fromStdString(m_fileOps->lastError());
            spdlog::error("Failed to load startup file '{}'", filePath);
            emit sig_loadWarning(
                QObject::tr("Не удалось загрузить файл:\n%1\n%2")
                    .arg(qFilePath, detail));
        } else {
            spdlog::info("Startup file loaded: {}", filePath);
        }
    }
}