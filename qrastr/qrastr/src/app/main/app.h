#pragma once

#include <QApplication>
#include <QDir>
#include <QObject>
#include <spdlog/spdlog.h>
#include "cacheLog.h"

class QAstra;
class QTI;
class QBarsMDP;
class CUIFormsCollection;
class CUIForm;

/**
 * @class App
 * Отвечает за:
 * - Инициализацию всей системы
 * - Загрузку плагинов для расчётов
 * - Управление настройками
 * - Настройку системы логирования
 */
class App final : public QApplication{
    Q_OBJECT
public:

    App(int &argc, char **argv);
    ~App() override;
    // Переопределённые методы Qt
    ///@brief Обработка событий Qt
    bool event( QEvent *event ) override;
    ///@brief  Глобальная обработка исключений
    bool notify(QObject* receiver, QEvent* event) override;

    // Методы жизненного цикла
    long readSettings(); // Чтение appsettings.json
    long writeSettings(); // Сохранение настроек
    long init(); // Инициализация логирования
    long start(); // Загрузка плагинов и данных

    // Загрузка компонентов
    void loadPlugins(); // Динамическая загрузка .dll/.so плагинов
    // form files are deployed in form catalog near qrastr.exe
    long readForms(); // Чтение описаний форм (.fm файлы)

    // Геттеры для доступа к плагинам
    std::list<CUIForm>& getForms() const;
    std::shared_ptr<QAstra> getQAstraPtr(){ return m_sp_qastra;}
    std::shared_ptr<QTI> getQTIPtr(){ return m_sp_qti;}
    std::shared_ptr<QBarsMDP> getQBarsMDPPtr(){ return m_sp_qbarsmdp;}

private:
    // Кэш логов до инициализации
    qrastr::CacheLogVector m_v_cache_log;

    // Умные указатели на плагины
    std::shared_ptr<QAstra> m_sp_qastra;
    std::shared_ptr<QTI> m_sp_qti;
    std::shared_ptr<QBarsMDP> m_sp_qbarsmdp;
    // Коллекция форм для отображения данных
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
};
