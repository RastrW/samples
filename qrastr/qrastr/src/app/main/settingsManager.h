#pragma once

#include <QObject>
#include <QSettings>
#include <QMainWindow>
#include <QVariant>
#include <QString>
#include <memory>

#include <spdlog/spdlog.h>

namespace qrastr {
    class CacheLogVector;
}

class QAstra;

/// @class Менеджер настроек приложения
class SettingsManager : public QObject {
    Q_OBJECT
    
public:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() = default;
    
    // ========== Настройки окна ==========
    /// @brief Загрузить геометрию окна
    /// it cache log messages to vector, because it called befor logger intialization
    bool loadWindowGeometry(QMainWindow* window);
    /// @brief Сохранить геометрию окна
    bool saveWindowGeometry(QMainWindow* window);
    
    // ========== Кэширование логов ==========
    /// @brief Вывести кэшированные логи
    void flushLogCache();
    // ========== Настройки форм ==========
    /// @brief Показать диалог настроек форм
    void showFormSettings(std::shared_ptr<QAstra> qastra);
private:
    QSettings
        m_settings;
    std::unique_ptr<qrastr::CacheLogVector>
        m_logCache;
};
