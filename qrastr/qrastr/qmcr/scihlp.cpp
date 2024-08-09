#include <QLibrary>
#include <QMessageBox>
#include "SciLexer.h"
#include "scihlp.h"

SciHlp::SciHlp(QWidget *parent, _en_role role)
    : ScintillaEdit(parent)
    , role_(role){
}
void SciHlp::setStyleHlp(sptr_t style, sptr_t fore, bool bold, bool italic, sptr_t back, bool underline, bool eolfilled){
    styleSetFore      ( style, fore      );
    styleSetBold      ( style, bold      );
    styleSetItalic    ( style, italic    );
    styleSetBack      ( style, back      );
    styleSetUnderline ( style, underline );
    styleSetEOLFilled ( style, eolfilled );
}
void SciHlp::showEvent(QShowEvent *event){
    #if _WIN32 //https://www.scintilla.org/LexillaDoc.html
        typedef void *(__stdcall *CreateLexerFn)(const char *name);
        const QFunctionPointer pfn = QLibrary::resolve("lexilla5", "CreateLexer");
    #else
        typedef void *(*CreateLexerFn)(const char *name);
        const QFunctionPointer pfn = QLibrary::resolve("libLexilla.so.5", "CreateLexer");
    #endif
    if(pfn == nullptr){
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString("error: invalid get function lexilla5.CreateLexer")
                       );
        mb.exec();
        return;
    }
    const std::string strLanguageName {"python"};
    const void* plex = ( reinterpret_cast<CreateLexerFn>(pfn) )( strLanguageName.c_str() );
    if(plex == nullptr){
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString("error: invalid lexx [%1] pointer").arg(strLanguageName.c_str())
                       );
        mb.exec();
        return;
    }
    setILexer( reinterpret_cast<sptr_t>(plex) );
    setCodePage(SC_CP_UTF8);
    //fol symbols
    markerDefine(SC_MARKNUM_FOLDEROPEN,    SC_MARK_MINUS);
    markerDefine(SC_MARKNUM_FOLDER,        SC_MARK_PLUS);
    markerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_MINUS);
    markerDefine(SC_MARKNUM_FOLDEREND,     SC_MARK_PLUS);
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


/*
#define SCE_P_DEFAULT 0
#define SCE_P_COMMENTLINE 1
#define SCE_P_NUMBER 2
#define SCE_P_STRING 3
#define SCE_P_CHARACTER 4
#define SCE_P_WORD 5
#define SCE_P_TRIPLE 6
#define SCE_P_TRIPLEDOUBLE 7
#define SCE_P_CLASSNAME 8
#define SCE_P_DEFNAME 9
#define SCE_P_OPERATOR 10
#define SCE_P_IDENTIFIER 11
#define SCE_P_COMMENTBLOCK 12
#define SCE_P_STRINGEOL 13
#define SCE_P_WORD2 14
#define SCE_P_DECORATOR 15
#define SCE_P_FSTRING 16
#define SCE_P_FCHARACTER 17
#define SCE_P_FTRIPLE 18
#define SCE_P_FTRIPLEDOUBLE 19
#define SCE_P_ATTRIBUTE 20
*/
    //setStyleHlp( SCE_P_DEFAULT, _colors::blue, true );

    setStyleHlp( SCE_P_COMMENTLINE ,  _colors::green );      // #xxx
    setStyleHlp( SCE_P_NUMBER ,       _colors::red, true);   // 13
    setStyleHlp( SCE_P_STRING ,       _colors::teal   );      // ""xxx""
    setStyleHlp( SCE_P_CHARACTER ,    _colors::teal   );      // 'xxx'
    setStyleHlp( SCE_P_WORD ,         _colors::maroon, true ); // for xxx in :
    setStyleHlp( SCE_P_TRIPLE ,       _colors::fuchsia );    // ?
    setStyleHlp( SCE_P_TRIPLEDOUBLE , _colors::green   );      // """
    setStyleHlp( SCE_P_CLASSNAME ,    _colors::blue,  true );
    setStyleHlp( SCE_P_DEFNAME ,      _colors::navy, true ); // def xxx():
    setStyleHlp( SCE_P_OPERATOR ,     _colors::black   );       // xxx =
    setStyleHlp( SCE_P_IDENTIFIER ,   _colors::black   );       // xxx =
    setStyleHlp( SCE_P_COMMENTBLOCK , _colors::green   );       // #
    setStyleHlp( SCE_P_STRINGEOL ,    _colors::maroon  );        // ?
    setStyleHlp( SCE_P_WORD2 ,        _colors::navy    );        // ?
    setStyleHlp( SCE_P_DECORATOR ,    _colors::olive,  true );  // @gfg_decorator
    setStyleHlp( SCE_P_FSTRING ,      _colors::teal, true );
    setStyleHlp( SCE_P_FCHARACTER ,   _colors::teal   );
    setStyleHlp( SCE_P_FTRIPLE ,      _colors::green    );   // f''' '''
    setStyleHlp( SCE_P_FTRIPLEDOUBLE, _colors::green    );   // f""" """
    setStyleHlp( SCE_P_ATTRIBUTE ,    _colors::purple   );


    //setStyleHlp( SCE_P_WORD, getRGBA( 0x31, 0x8c, 0xe7), true );

/*  setStyleHlp( SCE_C_WORD, getRGBA( 0x31, 0x8c, 0xe7), true );
    setStyleHlp( SCE_C_COMMENT, _colors::green );
    setStyleHlp( SCE_C_COMMENTLINE, _colors::green );//&
    setStyleHlp( SCE_C_COMMENTDOC, _colors::green );//&
    setStyleHlp( SCE_C_NUMBER, _colors::red );//&
*/
    /*


    SCE_C_STRING
    SCE_C_CHARACTER
    SCE_C_UUID
    SCE_C_OPERATOR
    SCE_C_PREPROCESSOR
    */
    return;

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

    styleSetFore      (styleNumber, getRGBA( 0x31, 0x8c, 0xe7));
    styleSetBack      (styleNumber, _colors::white);
    styleSetBold      (styleNumber, true);
    styleSetItalic    (styleNumber, false);
    styleSetUnderline (styleNumber, false);
    styleSetEOLFilled (styleNumber, true);
    /*
    SetAStyle(SCE_C_COMMENT, ini.GetColor(_T("comment")));
    SetAStyle(SCE_C_COMMENTLINE, ini.GetColor(_T("comment")));
    SetAStyle(SCE_C_COMMENTDOC, ini.GetColor(_T("comment")));
    SetAStyle(SCE_C_NUMBER, ini.GetColor(_T("number")));
    SetAStyle(SCE_C_STRING, ini.GetColor(_T("string")));
    SetAStyle(SCE_C_CHARACTER, ini.GetColor(_T("string")));
    SetAStyle(SCE_C_UUID, ini.GetColor(_T("uuid")));
    SetAStyle(SCE_C_OPERATOR, ini.GetColor(_T("operators")));
    SetAStyle(SCE_C_PREPROCESSOR, ini.GetColor(_T("preprocessor")));
    SetAStyle(SCE_C_WORD, ini.GetColor(_T("keywords")));
    //SetAStyle(SCE_C_WORD2, ini.GetColor(_T("keywords")));
*/
    //dockeyword = #008000


};
