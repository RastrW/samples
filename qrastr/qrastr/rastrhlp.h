#ifndef CRASTRHLP_H
#define CRASTRHLP_H
#include "common.h"
#include "astra_exp.h"

class CRastrHlp
{
public:
    CRastrHlp();
    virtual ~CRastrHlp();
    int CreateRastr();
    int ReadForms(std::string str_path_to_file_forms);

    static bool IsIdValid(_idRastr id );


    static constexpr long SIZE_STR_BUF_ = 100'000;
private:
    _idRastr id_rastr_ = -1;
};

#endif // CRASTRHLP_H
