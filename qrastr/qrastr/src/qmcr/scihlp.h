#pragma once

#include <QFileInfo>
#include "ScintillaEdit.h"


class SciHlp
    : public ScintillaEdit {
    Q_OBJECT
public:
    enum class Role   { EditorPython, ProtocolLog };
    enum class RetVal { Ok = 1, Failure = -1 };
    //https://www.scintilla.org/ScintillaDoc.html#colour
    struct FindParams {
        explicit FindParams(const QString& text) : m_text(text) {}
        QString m_text;
    };
    SciHlp(QWidget *parent, Role role);
    ~SciHlp() override = default;

    RetVal setContent(const std::string& str_text);
    RetVal appendTextCustom(const std::string_view svTxt);
    bool isModified() const;
    RetVal setFileInfo(const QFileInfo& fiNew);
    const QFileInfo& getFileInfo()const;
    RetVal saveToFile();
    RetVal loadFromFile();
    /**
     * @brief find
     * - SCFIND_REGEXP	    The search string should be interpreted as
     * a regular expression. Uses Scintilla's base implementation unless combined with SCFIND_CXX11REGEX.
     * - SCFIND_POSIX	    Treat regular expression in a more POSIX compatible
     * manner by interpreting bare ( and ) for tagged sections rather than \( and \).
     * Has no effect when SCFIND_CXX11REGEX is set.
     * - SCFIND_CXX11REGEX	This flag may be set to use C++11 <regex> instead of
     * Scintilla's basic regular expressions. If the regular expression is
     * invalid then -1 is returned and status is set to SC_STATUS_WARN_REGEX.
     * The ECMAScript flag is set on the regex object and UTF-8 documents will
     * exhibit Unicode-compliant behaviour. For MSVC, where wchar_t is 16-bits,
     * the regular expression ".." will match a single astral-plane character.
     * There may be other differences between compilers. Must also have SCFIND_REGEXP set.
    */
    RetVal find(FindParams params_find);

    const char* monospaceFontName();
    void showEvent(QShowEvent *event) override;
signals:
    void sig_fileInfoChanged(const QFileInfo& fiNew);
private slots:
    void slot_marginClicked(Scintilla::Position position,
                            Scintilla::KeyMod modifiers, int margin);
    void slot_notify(Scintilla::NotificationData* pnd);
private:
    void showAllLexer();
    void setStyleHlp(sptr_t style, sptr_t fore,
                     bool bold = false,
                     bool italic = false,
                     sptr_t back = _colors::white,
                     bool underline = false,
                     bool eolfilled = false);

    static constexpr unsigned long getRGBA(const std::uint8_t r, const std::uint8_t g,
                                           const std::uint8_t b, const std::uint8_t a ){
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

    QFileInfo m_fileInfo;

    const sptr_t k_marginLineNum = 0;
    const sptr_t k_marginFold     = 1;
};
