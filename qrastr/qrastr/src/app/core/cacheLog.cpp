#include "cacheLog.h"

using namespace qrastr;

CacheLog::CacheLog( const spdlog::level::level_enum lev_in, std::string_view sv_in )
    : lev{lev_in}
    , str_log{sv_in}{
}

CacheLog& 
CacheLog::CacheLog::operator=(const CacheLog& cache_log){
    lev     = cache_log.lev;
    str_log = cache_log.str_log;
    return *this;
}

CacheLog& 
CacheLog::operator=(const CacheLog&& cache_log){
    operator=(cache_log);
    return *this;
}

CacheLog::CacheLog(const CacheLog& cache_log){
    operator=(cache_log);
}

CacheLog::CacheLog(const CacheLog&& cache_log){
    operator=(cache_log);
}

void CacheLogVector::flush() {
	for (const auto& log : *this) {
        spdlog::log(log.lev, log.str_log);
	}
	clear();
}

void CacheLogVector::flushToSinks(
    std::initializer_list<std::shared_ptr<spdlog::sinks::sink>> target_sinks)
{
    auto logger_name = spdlog::default_logger()->name(); // "qrastr"
    for (const auto& entry : *this) {
        spdlog::details::log_msg msg(
            spdlog::source_loc{},
            logger_name,
            entry.lev,
            entry.str_log
            );
        for (auto& sink : target_sinks) {
            if (sink->should_log(entry.lev)) {
                sink->log(msg);
                sink->flush();
            }
        }
    }
    // НЕ очищаем *this — кэш нужен для последующего воспроизведения в Qt
}