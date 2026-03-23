#pragma once

#include <QObject>
#include <QSettings>
#include <memory>

class QAstra;
class QMainWindow;

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
    void saveValue(const QString& key, const QByteArray& value);
    // ========== Настройки форм ==========
    /// @brief Показать диалог настроек форм
    void showFormSettings(std::shared_ptr<QAstra> qastra); 
    QByteArray getSettings(const QString& name);
private:
    QSettings
        m_settings;
};
