#ifndef SCIHLP_H
#define SCIHLP_H
#pragma once

#include <QFileInfo>
//#include "C:\Projects\compile\sci\scintilla\qt\ScintillaEdit\ScintillaEdit.h"
//#include <ScintillaEdit.h>
#include "ScintillaEdit.h"

//#if defined(Q_OS_WIN)

class SciHlp
    : public ScintillaEdit {
    Q_OBJECT
public:
    enum class _en_role{
        editor_python = 0,
        prot_macro    = 1
    };
    enum class _ret_vals{
        ok      = 1,
        failure = -1
    };
    //https://www.scintilla.org/ScintillaDoc.html#colour
    static constexpr unsigned long getRGBA( const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a ){
        assert(a == 0x00);//because is not tested!
        return ( r + (g << 8) + (b << 16) + (a << 24) );
    }
    struct _colors{ // https://github.com/ubernostrum/webcolors/blob/trunk/src/webcolors/_definitions.py
        typedef unsigned long _color;
        inline static _color aqua    { getRGBA( 0x00, 0xff, 0xff, 0x00 ) };
        inline static _color black   { getRGBA( 0x00, 0x00, 0x00, 0x00 ) };
        inline static _color blue    { getRGBA( 0x00, 0x00, 0xff, 0x00 ) };
        inline static _color fuchsia { getRGBA( 0xff, 0x00, 0xff, 0x00 ) };
        inline static _color green   { getRGBA( 0x00, 0x80, 0x00, 0x00 ) };
        inline static _color gray    { getRGBA( 0x80, 0x80, 0x80, 0x00 ) };
        inline static _color lime    { getRGBA( 0x00, 0xff, 0x00, 0x00 ) };
        inline static _color maroon  { getRGBA( 0x80, 0x00, 0x00, 0x00 ) };
        inline static _color navy    { getRGBA( 0x00, 0x00, 0x80, 0x00 ) };
        inline static _color olive   { getRGBA( 0x80, 0x80, 0x00, 0x00 ) };
        inline static _color purple  { getRGBA( 0x80, 0x00, 0x80, 0x00 ) };
        inline static _color red     { getRGBA( 0xff, 0x00, 0x00, 0x00 ) };
        inline static _color silver  { getRGBA( 0xc0, 0xc0, 0xc0, 0x00 ) };
        inline static _color teal    { getRGBA( 0x00, 0x80, 0x80, 0x00 ) };
        inline static _color white   { getRGBA( 0xff, 0xff, 0xff, 0x00 ) };
        inline static _color yellow  { getRGBA( 0xff, 0xff, 0x00, 0x00 ) };
    };
    struct _params_find{
        _params_find(const QString& qstrFind)
            :qstrFind_{qstrFind}{
        }
        const QString& qstrFind_;
    };
    SciHlp(QWidget *parent, _en_role role);
    virtual ~SciHlp() = default;
    void tstSci();
    const char* MonospaceFont();
    void showEvent(QShowEvent *event) override;
    void setStyleHlp( sptr_t style, sptr_t fore, bool bold=false, bool italic=false, sptr_t back=_colors::white, bool underline=false, bool eolfilled=false );
    _ret_vals setContent(const std::string& str_text);
    _ret_vals my_appendText(const std::string_view svTxt);
    bool getContentModified() const;
    _ret_vals setFileInfo(const QFileInfo& fiNew);
    const QFileInfo& getFileInfo()const;
    _ret_vals ContentToFile();
    _ret_vals ContentFromFile();
    _ret_vals Find(_params_find params_find);
signals:
    void chngFileInfo(const QFileInfo& fiNew);
private slots:
    void onMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
    void onNotify(Scintilla::NotificationData* pnd);
private:
    const _en_role role_;
    const sptr_t margin_line_num_ = 0;
    const sptr_t margin_fold_     = 1;
    QFileInfo fiFileSource_;
};//class SciHlp{

#endif // SCIHLP_H
