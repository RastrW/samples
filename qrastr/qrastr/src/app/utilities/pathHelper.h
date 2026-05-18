#pragma once

#include <QString>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>
#include <spdlog/spdlog.h>

class PathHelper
{
public:
    /// Возвращает абсолютный путь к папке Data
    /// В AppImage: /path/to/.AppImage/../Data
    /// При разработке: applicationDirPath()/../Data
    static QString getDataPath()
    {
        const QString appImageEnv = QString::fromStdString(
            std::getenv("APPIMAGE") ? std::getenv("APPIMAGE") : "");

        QString baseDir;
        if (!appImageEnv.isEmpty()) {
            // Запуск из AppImage
            baseDir = QFileInfo(appImageEnv).absolutePath();
        } else {
            // Разработка (QtCreator)
            baseDir = QDir::cleanPath(
                QCoreApplication::applicationDirPath() + "/..");
        }

        return baseDir;
    }

    /// Путь к конкретному ресурсу в Data
    static QString getDataFile(const QString& relativePath)
    {
        return QDir::cleanPath(getDataPath() + "/Data/" + relativePath);
    }

    /// Путь к графическим ресурсам
    static QString getGraphicsPath(const QString& filename)
    {
        return getDataFile("graphics/" + filename);
    }

    /// Путь к макросам
    static QString getMacroPath(const QString& filename)
    {
        return getDataFile("contextmacro/" + filename);
    }

    /// Путь к модулям Python
    static QString getPythonModulePath(const QString& moduleName)
    {
#ifdef _DEBUG
        return getDataFile("astr_py/Debug/" + moduleName);
#else
        return getDataFile("astr_py/Release/" + moduleName);
#endif
    }

    /// Логирование найденного пути (для отладки)
    static void logPath(const QString& description, const QString& path)
    {
        spdlog::debug("PathHelper [{}]: {}",
                      description.toStdString(),
                      path.toStdString());

        if (!QFileInfo::exists(path)) {
            spdlog::warn("PathHelper: path not found: {}",
                         path.toStdString());
        }
    }
};
