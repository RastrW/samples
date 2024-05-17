#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include "fmt/format.h"


enum class _err_code{
    norm = 1,
    fail = -1,
};

template <typename... Args>
//void ppl( _err_code eCod, std::string_view sv_format, Args&&... args )
void ppl( _err_code eCod, std::string sv_format, Args&&... args )
{
    //
    fmt::format(sv_format, args...) ;
    //qDebug() << str_log;
}


template <typename... Args>
static void plog( _err_code eCod, std::string_view sv_format, Args&&... args ){
    const std::string str_log{fmt::format(sv_format, args...)};
    qDebug() << str_log;
};


#endif // COMMON_H
