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
