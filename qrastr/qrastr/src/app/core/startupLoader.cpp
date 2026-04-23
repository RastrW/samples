#include "startupLoader.h"

#include "rastrParameters.h"
#include <spdlog/spdlog.h>
#include <QMessageBox>
#include "qastra.h"

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
    const QDir templatesDir =
        RastrParameters::get_instance()->getDirSHABLON();

    if (!loadTemplates(templatesDir))
        return false;

    loadFiles(templatesDir); // некритично — всегда true, ошибки через сигнал
    return true;
}

bool StartupLoader::loadTemplates(const QDir& templatesDir) {
    const auto& params = RastrParameters::get_instance();

    for (const auto& templateName : params->getStartLoadTemplates()) {
        const QString fullPath =
            templatesDir.filePath(QString::fromStdString(templateName));

        const IPlainRastrRetCode res =
            m_qastra->Load(eLoadCode::RG_REPL, "", fullPath.toStdString());

        if (res != IPlainRastrRetCode::Ok) {
            spdlog::error("Failed to load template: {}", templateName);
            QMessageBox::critical(
                m_parentWidget, QObject::tr("Ошибка"),
                QString(QObject::tr("Не удалось загрузить шаблон:\n%1"))
                    .arg(QString::fromStdString(templateName)));
            return false; // шаблон критичен
        }
        spdlog::info("Template loaded: {}", templateName);
    }
    return true;
}

bool StartupLoader::loadFiles(const QDir& templatesDir) {
    const auto& fileList = RastrParameters::get_instance()->getStartLoadFileTemplates();

    for (const auto& [filePath, templateName] : fileList) {
        // Проверяем существование до загрузки
        const QString qFilePath = QString::fromStdString(filePath);
        if (!QFileInfo::exists(qFilePath)) {
            //Нет файла - информируем пользователя и идем дальше.
            spdlog::warn("Startup file not found: {}", filePath);
            const QString msg = QString("Файл не найден:\n%1\n\n").arg(qFilePath);
            emit loadWarning(msg);
            QMessageBox::warning(m_parentWidget, QObject::tr("Файл не найден"), msg);
            continue;
        }
        try {
            const QString fullTemplatePath =
                templatesDir.filePath(QString::fromStdString(templateName));

            const IPlainRastrRetCode res = m_qastra->Load(
                eLoadCode::RG_REPL,
                filePath,
                fullTemplatePath.toStdString());

            if (res == IPlainRastrRetCode::Ok)
                spdlog::info("Startup file loaded: {}", filePath);
            else {
                spdlog::error("Failed to load startup file: {} (code {})",
                              filePath, static_cast<int>(res));
                emit loadWarning(
                    QString(QObject::tr("Не удалось загрузить файл:\n%1"))
                        .arg(qFilePath));
            }
        } catch (const std::exception& ex) {
            spdlog::error("Exception loading startup file {}: {}", filePath, ex.what());
            QMessageBox::critical(
                m_parentWidget, QObject::tr("Ошибка"),
                QString(QObject::tr("Исключение при загрузке файла:\n%1"))
                    .arg(qFilePath));
        }
    }
    return true;
}