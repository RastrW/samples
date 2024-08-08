#include <QLibrary>
#include <QMessageBox>
#include "SciLexer.h"
#include "scihlp.h"

//https://www.scintilla.org/ScintillaDoc.html#colour
constexpr unsigned long getRGBA( const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a = 0x00 ){
    assert(a == 0x00);//because is not tested!
    return ( r + (g << 8) + (b << 16) + (a << 24) );
}
/* // https://github.com/ubernostrum/webcolors/blob/trunk/src/webcolors/_definitions.py
"aqua": "#00 ff ff",
"black": "#00 00 00",
"blue": "#00 00 ff",
"fuchsia": "#ff 00 ff",
"green": "#00 80 00",
"gray": "#80 80 80",
"lime": "#00 ff 00",
"maroon": "#80 00 00",
"navy": "#00 00 80",
"olive": "#80 80 00",
"purple": "#80 00 80",
"red": "#ff 00 00",
"silver": "#c0 c0 c0",
"teal": "#00 80 80",
"white": "#ff ff ff",
"yellow": "#ff ff 00",
*/
struct _colors{
    typedef unsigned long _color;
    static constexpr _color aqua    { getRGBA( 0x00, 0xff, 0xff ) };
    static constexpr _color black   { getRGBA( 0x00, 0x00, 0x00 ) };
    static constexpr _color blue    { getRGBA( 0x00, 0x00, 0xff ) };
    static constexpr _color fuchsia { getRGBA( 0xff, 0x00, 0xff ) };
    static constexpr _color green   { getRGBA( 0x00, 0x80, 0x00 ) };
    static constexpr _color gray    { getRGBA( 0x80, 0x80, 0x80 ) };
    static constexpr _color lime    { getRGBA( 0x00, 0xff, 0x00 ) };
    static constexpr _color maroon  { getRGBA( 0x80, 0x00, 0x00 ) };
    static constexpr _color navy    { getRGBA( 0x00, 0x00, 0x80 ) };
    static constexpr _color olive   { getRGBA( 0x80, 0x80, 0x00 ) };
    static constexpr _color purple  { getRGBA( 0x80, 0x00, 0x80 ) };
    static constexpr _color red     { getRGBA( 0xff, 0x00, 0x00 ) };
    static constexpr _color silver  { getRGBA( 0xc0, 0xc0, 0xc0 ) };
    static constexpr _color teal    { getRGBA( 0x00, 0x80, 0x80 ) };
    static constexpr _color white   { getRGBA( 0xff, 0xff, 0xff ) };
    static constexpr _color yellow  { getRGBA( 0xff, 0xff, 0x00 ) };
};

SciHlp::SciHlp(QWidget *parent, _en_role role)
    : ScintillaEdit(parent)
    , role_(role){
}
void SciHlp::showEvent(QShowEvent *event){
    setCodePage(SC_CP_UTF8);

    //fol symbols
    markerDefine(SC_MARKNUM_FOLDEROPEN,    SC_MARK_MINUS);
    markerDefine(SC_MARKNUM_FOLDER,        SC_MARK_PLUS);
    markerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_MINUS);
    markerDefine(SC_MARKNUM_FOLDEREND,     SC_MARK_PLUS);

//https://www.scintilla.org/LexillaDoc.html
#if _WIN32
            typedef void *(__stdcall *CreateLexerFn)(const char *name);
            QFunctionPointer pfn = QLibrary::resolve("lexilla5", "CreateLexer");
#else
            typedef void *(*CreateLexerFn)(const char *name);
            QFunctionPointer pfn = QLibrary::resolve("libLexilla.so.5", "CreateLexer");
#endif
           //void *lexCpp = ((CreateLexerFn)fn)("cpp");
            //setILexer( reinterpret_cast<sptr_t>(lexCpp)); //setILexer((sptr_t)(void *)lexCpp);
            if(pfn == nullptr){
                QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                QString("error: invalid get function lexilla5.CreateLexer")
                               );
                mb.exec();
                return;
            }
            std::string strLanguageName {"python"};
            //void *lexPy = ((CreateLexerFn)fn)("python");

            void* plex = ( reinterpret_cast<CreateLexerFn>(pfn) )( strLanguageName.c_str() );

            //ILexer5* plex = CreateLexer(strLanguageName.c_str());
            //ILexer5* plex = CreateLexer("python");
            if(plex == nullptr){
                QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                                QString("error: invalid lexx [%1] pointer").arg(strLanguageName.c_str())
                               );
                mb.exec();
                return;
            }
            setILexer( reinterpret_cast<sptr_t>(plex) );
            setKeyWords(0,R"(
False None True and as assert break class continue def del elif else
except finally for from global if import in is lambda nonlocal not
or pass raise return try while with yield

abs aiter all anext any ascii bin bool breakpoint
bytearray bytes callable chr classmethod compile
complex delattr dict dir divmod enumerate eval
exec filter float format frozenset getattr globals
hasattr hash help hex id input int isinstance
issubclass iter len list locals map max memoryview
min next object oct open ord pow print property
range repr reversed round set setattr slice sorted
staticmethod str sum super tuple type vars zip _
__import__
)");

            typedef const int COLORREF;
            const COLORREF red = getRGBA(0xFF, 0, 0, 0);
            const COLORREF offWhite   = getRGBA(0xFF, 0xFB, 0xF0, 0);
            const COLORREF darkGreen = getRGBA(0, 0x80, 0, 0);
            const COLORREF darkBlue  = getRGBA (0x00, 0x00, 0x80, 0);

            //const COLORREF xz = getRGB( 0x31, 0x8c, 0xe7, 0xff); // const QString str_keyword_color = "#31 8c e7";
            //const COLORREF xz = getRGB( 0xFF, 0x00, 0x00, 0x0); // const QString str_keyword_color = "#31 8c e7";
            const COLORREF xz = offWhite;

            setProperty("fold", "1");
            setProperty("fold.compact", "0");

            const int styleNumber = SCE_C_WORD; // keyword
            const QString str_keyword_color = "#318ce7";

            const int n_color = xz;

            styleSetFore      (styleNumber, _colors::yellow);
            styleSetBack      (styleNumber, _colors::white);
            styleSetBold      (styleNumber, true);
            styleSetItalic    (styleNumber, false);
            styleSetUnderline (styleNumber, false);
            styleSetEOLFilled (styleNumber, true);

            //dockeyword = #008000


};
