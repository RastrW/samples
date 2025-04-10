#ifndef COMMON_QRASTR_H
#define COMMON_QRASTR_H
#pragma once

#include <QDebug>
#include <string>
#include <exception>
//#undef SPDLOG_USE_STD_FORMAT
//#define FMT_HEADER_ONLY
//#define SPDLOG_USE_STD_FORMAT
//#define SPDLOG_USE_WCHAR
#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>
//#include <spdlog/fmt/bundled/format.h>
#include "Exceptions.h"

enum class _err_code1{
    norm = 1,
    fail = -1,
};

template <typename... Args>
static void plog1( const _err_code1 eCod, const std::string_view sv_format, const Args&&... args ){
    //  const std::string str_log{fmt::format(sv_format, args...)};
    const std::string str_log{"disabled"};
    spdlog::source_loc loc;
    spdlog::error(sv_format, args...);
    qDebug() << str_log.c_str();
}

static void exclog(const std::exception& ex){
    spdlog::error("Catch exception [{}]\n", ex.what());
}

static void exclog(){
    spdlog::error("Catch unknown exception.");
}

static void exclog(const CException& ex){
    spdlog::error("Catch CException [{}]\n", ex.what());
}

#endif // COMMON_QRASTR_H
