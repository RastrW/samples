#pragma once

#include <QApplication>
#include <QDir>
#include <QObject>
#include <spdlog/spdlog.h>

namespace qrastr {
	/**
     * @brief Структура для кэширования логов до инициализации системы логирования
    */
	struct CacheLog {
        spdlog::level::level_enum lev;
        std::string               str_log;
        CacheLog( const spdlog::level::level_enum lev_in, std::string_view sv_in );
        CacheLog& operator=(const CacheLog&  cache_log);
        CacheLog& operator=(const CacheLog&& cache_log);
        CacheLog           (const CacheLog&  cache_log);
        CacheLog           (const CacheLog&& cache_log);
	};

	/**
     * @brief Вектор для накопления логов
     * Расширяет std::vector методом add() для удобного форматирования
    */
	class CacheLogVector : public std::vector<CacheLog> {
	public:
        template <typename... Args>
        void add(spdlog::level::level_enum lev,
                 std::string_view format,
                 Args&&... args) {
            emplace_back(lev, fmt::format(format, std::forward<Args>(args)...));
        }
		
		void flush();
	};
}
