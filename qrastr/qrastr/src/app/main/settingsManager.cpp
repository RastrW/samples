#include "settingsManager.h"
#include "qastra.h"
#include "settingsDialog.h"
#include "cacheLog.h"
#include <QMainWindow>
#include <spdlog/spdlog.h>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_logCache(std::make_unique<qrastr::CacheLogVector>())
{
}

bool SettingsManager::loadWindowGeometry(QMainWindow* window) {
    if (!window) {
        return false;
    }
    
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
        m_logCache->add(spdlog::level::err, "Exception: {}", ex.what());
        return false;
    }
    catch (...) {
        m_logCache->add(spdlog::level::err, "Unknown exception.");
        return false;
    }
    
    return true;
}

QByteArray SettingsManager::getSettings(const QString& name){
    return m_settings.value(name).toByteArray();
}

bool SettingsManager::saveWindowGeometry(QMainWindow* window) {
    if (!window) {
        return false;
    }
    
    m_settings.beginGroup("MainWindow");
    m_settings.setValue("geometry", window->saveGeometry());
    m_settings.endGroup();
    
    return true;
}

void SettingsManager::flushLogCache() {
    m_logCache->flush();
}

void SettingsManager::showFormSettings(std::shared_ptr<QAstra> qastra) {
    SettingsDialog* pformSettings = new SettingsDialog();
    pformSettings->init(qastra);
    pformSettings->setAttribute(Qt::WA_DeleteOnClose);
    pformSettings->show();
}
