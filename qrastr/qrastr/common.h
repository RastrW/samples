#ifndef COMMON_H
#define COMMON_H

#include <exception>
#include <QDebug>
#include "fmt/format.h"
#include "Exceptions.h"

enum class _err_code{
    norm = 1,
    fail = -1,
};

template <typename... Args>
static void plog( _err_code eCod, std::string_view sv_format, Args&&... args ){
    const std::string str_log{fmt::format(sv_format, args...)};
    qDebug() << str_log.c_str();
};

static void exclog(const std::exception& ex){
    plog(_err_code::fail, "got exception [{}]\n", ex.what());
}

static void exclog(){
    plog(_err_code::fail, "got unknown exception \n");
}

static void exclog(const CException& ex){
    plog(_err_code::fail, "got exception [{}]\n", ex.what());
}

#endif // COMMON_H
