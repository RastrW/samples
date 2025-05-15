#ifndef APP_H
#define APP_H
#pragma once

#include <QApplication>
#include <QDir>
#include <QObject>
#include <spdlog/spdlog.h>

//namespace spdlog{namespace level{enum level_enum;}}
class QAstra;
class QTI;
class CUIFormsCollection;
class CUIForm;
class App final
    : public QApplication{
    Q_OBJECT
public:
    struct _cache_log{
        spdlog::level::level_enum lev;
        std::string               str_log;
        _cache_log( const spdlog::level::level_enum lev_in, std::string_view sv_in );
        _cache_log& operator=(const _cache_log&  cache_log);
        _cache_log& operator=(const _cache_log&& cache_log);
        _cache_log           (const _cache_log&  cache_log);
        _cache_log           (const _cache_log&& cache_log);
    };
    struct _v_cache_log
        : public std::vector<_cache_log> {
        template <typename... Args>
        void add( const spdlog::level::level_enum lev_in, const std::string_view sv_format, Args&&... args );
    };
    App(int &argc, char **argv);
    ~App() override;
    bool event( QEvent *event ) override;
    bool notify(QObject* receiver, QEvent* event) override;
    long readSettings();
    long writeSettings();
    long init();
    void loadPlugins();
    long readForms();
    std::list<CUIForm>& GetForms() const;
    std::shared_ptr<QAstra> getQAstraPtr(){ return m_sp_qastra;}
    std::shared_ptr<QTI> getQTIPtr(){ return m_sp_qti;}
    long start();

    _v_cache_log v_cache_log_;
    std::shared_ptr<QAstra> m_sp_qastra;
    std::shared_ptr<QTI> m_sp_qti;
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
};

#endif // APP_H
