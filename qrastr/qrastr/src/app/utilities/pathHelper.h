#pragma once

#include <QString>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>
#include <spdlog/spdlog.h>

class PathHelper
{
public:
    /// Определяет контекст запуска
    enum class Context {
        AppImage,      // Запуск из .AppImage файла
        QtCreator,     // Разработка в QtCreator
        SystemInstall  // Системная установка
    };

    /// Возвращает текущий контекст запуска
    static Context getContext()
    {
        const QString appImageEnv = QString::fromStdString(
            std::getenv("APPIMAGE") ? std::getenv("APPIMAGE") : "");

        if (!appImageEnv.isEmpty()) {
            return Context::AppImage;
        }

        // В QtCreator обычно запускается из Release/ или Debug/ папки
        const QString appPath = QCoreApplication::applicationDirPath();
        if (appPath.contains("Release") || appPath.contains("Debug")) {
            return Context::QtCreator;
        }

        return Context::SystemInstall;
    }

    /// Возвращает абсолютный путь к папке Data
    static QString getDataPath()
    {
        Context ctx = getContext();

        if (ctx == Context::AppImage) {
            const QString appImageEnv = QString::fromStdString(
                std::getenv("APPIMAGE") ? std::getenv("APPIMAGE") : "");
            return QFileInfo(appImageEnv).absolutePath() + "/Data";
        }
        else if (ctx == Context::QtCreator) {
            // Из Release/ или Debug/ поднимаемся выше
            // qrastr/Release/ -> qrastr/Data/
            return QDir::cleanPath(
                QCoreApplication::applicationDirPath() + "/../Data");
        }
        else {
            // Системная установка: /usr/share/qrastr/Data/
            return "/usr/share/qrastr/Data";
        }
    }

    /// Путь к конкретному ресурсу в Data
    static QString getDataFile(const QString& relativePath)
    {
        return QDir::cleanPath(getDataPath() + "/" + relativePath);
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

    /// Путь к папке с модулем astra_py (БЕЗ Release/Debug)
    /// На всех платформах: Data/astra_py/
    static QString getAstraPyDir()
    {
        return getDataFile("astra_py");
    }

    /// Путь к конкретному файлу модуля astra_py
    static QString getAstraPyFile(const QString& filename)
    {
        return getAstraPyDir() + "/" + filename;
    }

    /// Определяет имя модуля astra_py для текущей платформы
    static QString getAstraPyModuleName()
    {
#ifdef _WIN32
        return "astra_py.cp311-win_amd64.pyd";  // или cp312 в зависимости от версии Python
#else
        return "astra_py.cpython-311-x86_64-linux-gnu.so";
#endif
    }

    /// Путь к файлу модуля astra_py для текущей платформы
    static QString getAstraPyModulePath()
    {
        return getAstraPyFile(getAstraPyModuleName());
    }

    /// Логирование пути (для отладки)
    static void logPath(const QString& description, const QString& path, bool isRequired = true)
    {
        const bool exists = QFileInfo::exists(path);

        if (exists) {
            spdlog::info("PathHelper [{}]: {}",
                         description.toStdString(),
                         path.toStdString());
        } else {
            const auto level = isRequired ? spdlog::level::warn : spdlog::level::info;
            spdlog::log(level, "PathHelper [{}]: path not found: {}",
                        description.toStdString(),
                        path.toStdString());
        }
    }

    /// Проверка, что путь существует
    static bool pathExists(const QString& path)
    {
        return QFileInfo::exists(path);
    }

    /// Логирование информации о контексте
    static void logContext()
    {
        Context ctx = getContext();
        const char* ctxStr = "";
        switch (ctx) {
        case Context::AppImage:     ctxStr = "AppImage"; break;
        case Context::QtCreator:    ctxStr = "QtCreator (Debug/Release)"; break;
        case Context::SystemInstall: ctxStr = "System Install"; break;
        }

        spdlog::info("PathHelper: Context = {}", ctxStr);
        spdlog::info("PathHelper: Data path = {}", getDataPath().toStdString());
    }
};
