#pragma once

#include <QApplication>
#include <QDir>
#include <QObject>
#include <spdlog/spdlog.h>

//namespace spdlog{namespace level{enum level_enum;}}
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
    /**
     * @struct _cache_log
     * @brief Структура для кэширования логов до инициализации системы логирования
     *
     * Используется потому что логирование настраивается ПОСЛЕ чтения настроек,
     * но логи нужны уже ПРИ чтении настроек
     */
    struct _cache_log{
        spdlog::level::level_enum lev;
        std::string               str_log;
        _cache_log( const spdlog::level::level_enum lev_in, std::string_view sv_in );
        _cache_log& operator=(const _cache_log&  cache_log);
        _cache_log& operator=(const _cache_log&& cache_log);
        _cache_log           (const _cache_log&  cache_log);
        _cache_log           (const _cache_log&& cache_log);
    };
    /**
     * @struct _v_cache_log
     * @brief Вектор для накопления логов
     *
     * Расширяет std::vector методом add() для удобного форматирования
     */
    struct _v_cache_log
        : public std::vector<_cache_log> {
        template <typename... Args>
        void add( const spdlog::level::level_enum lev_in, const std::string_view sv_format, Args&&... args );
    };

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
    long readForms(); // Чтение описаний форм (.fm файлы)

    // Геттеры для доступа к плагинам
    std::list<CUIForm>& getForms() const;
    std::shared_ptr<QAstra> getQAstraPtr(){ return m_sp_qastra;}
    std::shared_ptr<QTI> getQTIPtr(){ return m_sp_qti;}
    std::shared_ptr<QBarsMDP> getQBarsMDPPtr(){ return m_sp_qbarsmdp;}

    // Кэш логов до инициализации
    _v_cache_log v_cache_log_;

    // Умные указатели на плагины
    std::shared_ptr<QAstra> m_sp_qastra;
    std::shared_ptr<QTI> m_sp_qti;
    std::shared_ptr<QBarsMDP> m_sp_qbarsmdp;
    // Коллекция форм для отображения данных
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
};
