#include "settingsManager.h"
#include "qastra.h"
#include "settingsDialog.h"
#include <QMainWindow>
#include <QStyle>
#include <QStyleFactory>
#include <QApplication>
#include <spdlog/spdlog.h>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent){}

bool SettingsManager::loadWindowGeometry(QMainWindow* window) {
    if (!window) {
        return false;
    }

    // Временная диагностика
    qInfo() << "Settings file:" << m_settings.fileName();
    qInfo() << "All keys:" << m_settings.allKeys();

    QString savedStyle = m_settings.value("appStyle").toString();
    qInfo() << "SavedStyle:" << savedStyle;
    if (savedStyle.isEmpty()){
        savedStyle = "windows11";
    }
    QApplication::setStyle(QStyleFactory::create(savedStyle));

    try {
        m_settings.beginGroup("MainWindow");
        const auto geometry = m_settings.value("geometry", QByteArray()).toByteArray();
        
        if (geometry.isEmpty()) {
            window->setGeometry(200, 200, 800, 800);
        } else {
            window->restoreGeometry(geometry);
        }

        m_settings.endGroup();
    }
    catch (const std::exception& ex) {
        // Кэшируем ошибку (логгер может быть не инициализирован)
        spdlog::error("Exception: {}", ex.what());
        return false;
    }
    catch (...) {
        spdlog::error("Unknown exception.");
        return false;
    }
    
    return true;
}

void SettingsManager::saveValue(const QString& key, const QByteArray& value) {
    m_settings.setValue(key, value);
}

QByteArray SettingsManager::getSettings(const QString& name){
    return m_settings.value(name).toByteArray();
}

bool SettingsManager::saveWindowGeometry(QMainWindow* window) {
    if (!window) return false;

    m_settings.beginGroup("MainWindow");
    m_settings.setValue("geometry", window->saveGeometry());
    m_settings.endGroup();

    // objectName() может быть пустым, name() возвращает "fusion", "windows" и т.д.
    const QString styleName = QApplication::style()->objectName();
    m_settings.setValue("appStyle", styleName.isEmpty() ? "Fusion" : styleName);
    m_settings.setValue("mainWindowState", window->saveState());
    qInfo() << "StyleName:" << styleName;

    return true;
}

void SettingsManager::showFormSettings(std::shared_ptr<QAstra> qastra) {
    SettingsDialog* pformSettings = new SettingsDialog();
    pformSettings->init(qastra);
    pformSettings->setAttribute(Qt::WA_DeleteOnClose);
    pformSettings->show();
}
