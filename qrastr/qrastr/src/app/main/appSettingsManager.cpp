#include "appSettingsManager.h"
#include "settings/settingsDialog.h"
#include <QMainWindow>
#include <QStyle>
#include <QStyleFactory>
#include <QApplication>
#include <spdlog/spdlog.h>
#include "settingsKeys.h"

AppSettingsManager::AppSettingsManager(QObject* parent)
    : QObject(parent){}

AppSettingsManager::~AppSettingsManager() {
    m_settings.sync();
}

bool AppSettingsManager::loadAppearanceSettings(QMainWindow* window) {
    if (!window) {
        return false;
    }

    // Временная диагностика
    spdlog::info("Settings file: {}", m_settings.fileName().toStdString());
    spdlog::info("All keys: {}", m_settings.allKeys().join(", ").toStdString());

    QString savedStyle = m_settings.value(SK::MainWindow::appStyle).toString();
    spdlog::info("SavedStyle: {}", savedStyle.toStdString());
    if (savedStyle.isEmpty()){
        savedStyle = "windows11";
    }
    QApplication::setStyle(QStyleFactory::create(savedStyle));

    try {
        const auto geometry = m_settings.value(SK::MainWindow::geometry, QByteArray()).toByteArray();
        const auto state = m_settings.value(SK::MainWindow::state).toByteArray();

        if (geometry.isEmpty()) {
            window->setGeometry(200, 200, 800, 800);
        } else {
            window->restoreGeometry(geometry);
        }

        if (!state.isEmpty()) {
            window->restoreState(state);
        }
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

bool AppSettingsManager::saveAppearanceSettings(QMainWindow* window) {
    if (!window) return false;

    m_settings.setValue(SK::MainWindow::groupName,
                        window->saveGeometry());
    m_settings.setValue(SK::MainWindow::state,
                        window->saveState());

    const QString styleName = QApplication::style()->objectName();
    m_settings.setValue(SK::MainWindow::appStyle,
                        styleName.isEmpty() ? "Fusion" : styleName);
    return true;
}

void AppSettingsManager::saveValue(const QString& key, const QByteArray& value) {
    m_settings.setValue(key, value);
}

QByteArray AppSettingsManager::getSettings(const QString& name){
    return m_settings.value(name).toByteArray();
}

void AppSettingsManager::showFormSettings(std::shared_ptr<QAstra> qastra) {
    SettingsDialog* pformSettings = new SettingsDialog();
    pformSettings->init(qastra);
    pformSettings->setAttribute(Qt::WA_DeleteOnClose);
    pformSettings->show();
}
