#ifndef SCIHLP_H
#define SCIHLP_H

#include "ScintillaEdit.h"

class SciHlp
    : public ScintillaEdit {
    Q_OBJECT
public:
    enum class _en_role{
        editor_python = 0,
        prot_macro    = 1
    };
    enum class _ret_vals{
        ok = 1,
        failure = -1
    };
    SciHlp(QWidget *parent, _en_role role);
    virtual ~SciHlp() = default;
    void showEvent(QShowEvent *event) override;
    _ret_vals setContent(const std::string& str_text){
        setText(str_text.c_str());
        //emptyUndoBuffer();
        //setSavePoint();
        return _ret_vals::ok;
    };
    const _en_role role_;
};//class SciHlp{

#endif // SCIHLP_H
