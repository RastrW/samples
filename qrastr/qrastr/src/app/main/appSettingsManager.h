#pragma once

#include <QObject>
#include <QSettings>
#include <memory>

class QAstra;
class QMainWindow;

/// @class Менеджер настроек
/// Геометрия окна, стиль, рабочие области
/// Управление окном Параметров, через которое изменяется appsettings.json и QSettings
class AppSettingsManager : public QObject {
    Q_OBJECT
    
public:
    explicit AppSettingsManager(QObject* parent = nullptr);
    ~AppSettingsManager();
    
    bool loadAppearanceSettings(QMainWindow* window);
    bool saveAppearanceSettings(QMainWindow* window);
    void saveValue(const QString& key, const QByteArray& value);
    // ========== Настройки форм ==========
    /// @brief Показать диалог настроек форм
    void showFormSettings(std::shared_ptr<QAstra> qastra); 
    QByteArray getSettings(const QString& name);

    QSettings* settings() { return &m_settings; }
private:
    QSettings
        m_settings;
};
