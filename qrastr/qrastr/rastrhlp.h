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
    _idRastr GetRastrId() const { return id_rastr_; };
    int ReadForms(std::string str_path_to_file_forms);
    int Load(std::string str_path_to_file); //
    std::list<CUIForm> GetForms() const;
    CUIForm GetUIForm(size_t n_form_indx);
    int GetFormData(int n_form_indx);
    const nlohmann::json& GetJForms() {return jforms_;};
    static bool IsIdValid(_idRastr id);
    static constexpr long SIZE_STR_BUF_ = 1000000;
private:
    _idRastr id_rastr_ = -1;
    std::unique_ptr<CUIFormsCollection> upCUIFormsCollection_;
    std::vector<std::unique_ptr<CUIFormsCollection>> vupCUIFormsCollection_;
    nlohmann::json jforms_; // for Dima reverse ccompatibility
};

#endif // CRASTRHLP_H
