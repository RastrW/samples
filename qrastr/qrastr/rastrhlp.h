#ifndef CRASTRHLP_H
#define CRASTRHLP_H
#include "common.h"
#include "astra_exp.h"
#include "UIForms.h"

class CRastrHlp
{
public:
    CRastrHlp();
    virtual ~CRastrHlp();
    int CreateRastr();
    int ReadForms(std::string str_path_to_file_forms);
    int Load(std::string str_path_to_file); //
    std::list<CUIForm> GetForms() const;
    static bool IsIdValid(_idRastr id);
    static constexpr long SIZE_STR_BUF_ = 100'000;
private:
    _idRastr id_rastr_ = -1;
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
};

#endif // CRASTRHLP_H
