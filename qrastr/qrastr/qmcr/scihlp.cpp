#include <QLibrary>
#include <QMessageBox>
#include <QFontDatabase>
#include <QTextStream>
#include <iostream>
#include "SciLexer.h"
#include "scihlp.h"

//all of wrapped shit can be found in -> ScintillaEdit.cpp <-
//https://www.scintilla.org/PaneAPI.html
//interesting scintilla use https://github.com/SolarAquarion/wxglterm/tree/master/src/external_plugins
//https://github.com/mneuroth/SciTEQt
SciHlp::SciHlp(QWidget *parent, _en_role role)
    : ScintillaEdit(parent)
    , role_(role){
    //By default,
    //margin 0 is set to display line numbers, but is given a width of 0, so it is hidden.
    //Margin 1 is set to display non-folding symbols and is given a width of 16 pixels, so it is visible.
    //Margin 2 is set to display the folding symbols, but is given a width of 0, so it is hidden.
    //Of course, you can set the margins to be whatever you wish.
    //https://www.scintilla.org/ScintillaDoc.html#SCI_GETMARGINTYPEN
    //https://stackoverflow.com/questions/78522506/scintilla-will-not-highlight-or-codefold-in-my-c
    //https://www.purebasic.fr/english/viewtopic.php?t=68691
    //https://github.com/jacobslusser/ScintillaNET/issues/307
    styleSetFont(STYLE_DEFAULT, MonospaceFont());
    setMarginWidthN(margin_line_num_, 40);
    setMarginWidthN(margin_fold_,     20);
    setMarginMaskN(margin_fold_, SC_MASK_FOLDERS);
    setMarginTypeN(margin_fold_, SC_MARGIN_SYMBOL);
    setMarginSensitiveN(margin_fold_, true);
    markerDefine(SC_MARKNUM_FOLDER,        SC_MARK_BOXPLUS);
    markerDefine(SC_MARKNUM_FOLDEROPEN,    SC_MARK_BOXMINUS);
    markerDefine(SC_MARKNUM_FOLDERSUB,     SC_MARK_VLINE);
    markerDefine(SC_MARKNUM_FOLDERTAIL,    SC_MARK_LCORNER);
    markerDefine(SC_MARKNUM_FOLDEREND,     SC_MARK_BOXPLUSCONNECTED);
    markerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    markerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
    //setFoldFlags(SC_FOLDFLAG_LINEAFTER_CONTRACTED); // SC_FOLDFLAG_LINEAFTER_CONTRACTED- line after folded row
    //setAutomaticFold(true);
    setUseTabs(false); // translate TAB to spaces
    setTabIndents(true);
    setTabWidth(4); // set TAB size in spaces
    //setWrapIndentMode(SC_WRAPINDENT_DEEPINDENT);
    setWrapIndentMode(SC_WRAPINDENT_INDENT );
    markerSetBack(0,1);
    markerSetBack(1,1);

#if _WIN32 //https://www.scintilla.org/LexillaDoc.html
    typedef void *(__stdcall *CreateLexerFn)(const char* name);
    const QFunctionPointer pfn = QLibrary::resolve("lexilla5", "CreateLexer");
#else
    typedef void *(*CreateLexerFn)(const char* name);
    const QFunctionPointer pfn = QLibrary::resolve("libLexilla.so.5", "CreateLexer");
#endif
    if(pfn == nullptr){
        QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                        QString(tr("error: invalid get function lexilla5.CreateLexer"))
                       );
        mb.exec();
        return;
    }
    if(SciHlp::_en_role::editor_python == role){
        const std::string strLanguageName {"python"};
        //const std::string strLanguageName {"xml"};
        const void* plex = ( reinterpret_cast<CreateLexerFn>(pfn) )( strLanguageName.c_str() );
        if(plex == nullptr){
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString(tr("error: invalid lexx [%1] pointer").arg(strLanguageName.c_str()))
                           );
            mb.exec();
            return;
        }
        setILexer( reinterpret_cast<sptr_t>(plex) );
        setCodePage(SC_CP_UTF8);

        setProperty("fold", "1"); // show Folders!!
        setProperty("fold.compact", "0");
        setProperty("fold.quotes.python", "1");
        setAutomaticFold(SCI_SETAUTOMATICFOLD);

        connect(this, SIGNAL(marginClicked( Scintilla::Position, Scintilla::KeyMod, int ) ), this, SLOT(onMarginClicked( Scintilla::Position, Scintilla::KeyMod, int ) ) );
        connect(this, SIGNAL(notify       ( Scintilla::NotificationData*                ) ), this, SLOT(onNotify       ( Scintilla::NotificationData* )                ) );

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
        //setStyleHlp( SCE_P_DEFAULT, _colors::blue, true );
        setStyleHlp( SCE_P_COMMENTLINE ,  _colors::green   );      // #xxx
        setStyleHlp( SCE_P_NUMBER ,       _colors::red,    true);  // 13
        setStyleHlp( SCE_P_STRING ,       _colors::teal    );      // ""xxx""
        setStyleHlp( SCE_P_CHARACTER ,    _colors::teal    );      // 'xxx'
        setStyleHlp( SCE_P_WORD ,         _colors::maroon, true ); // for xxx in :
        setStyleHlp( SCE_P_TRIPLE ,       _colors::green   );        // ''' '''  - multiline comment
        setStyleHlp( SCE_P_TRIPLEDOUBLE , _colors::green   );      // """ """  - multiline comment
        setStyleHlp( SCE_P_CLASSNAME ,    _colors::blue,   true ); // class xxx:
        setStyleHlp( SCE_P_DEFNAME ,      _colors::navy,   true ); // def xxx():
        setStyleHlp( SCE_P_OPERATOR ,     _colors::black   );       // xxx =
        setStyleHlp( SCE_P_IDENTIFIER ,   _colors::black   );       // xxx =
        setStyleHlp( SCE_P_COMMENTBLOCK , _colors::green   );       // #
        setStyleHlp( SCE_P_STRINGEOL ,    _colors::maroon  );       // ?
        setStyleHlp( SCE_P_WORD2 ,        _colors::navy    );       // ?
        setStyleHlp( SCE_P_DECORATOR ,    _colors::olive,  true );  // @gfg_decorator
        setStyleHlp( SCE_P_FSTRING ,      _colors::teal,   true );  // f""
        setStyleHlp( SCE_P_FCHARACTER ,   _colors::teal    );
        setStyleHlp( SCE_P_FTRIPLE ,      _colors::green   );       // f''' '''
        setStyleHlp( SCE_P_FTRIPLEDOUBLE, _colors::green   );       // f""" """
        setStyleHlp( SCE_P_ATTRIBUTE ,    _colors::fuchsia );
    }else{
        tstSci();// debug: trace all lexers
        //const std::string strLanguageName {"hypertext"};
        const std::string strLanguageName {"xml"};
        const void* plex = ( reinterpret_cast<CreateLexerFn>(pfn) )( strLanguageName.c_str() );
        if(plex == nullptr){
            QMessageBox mb( QMessageBox::Icon::Critical, QObject::tr("Error"),
                            QString(tr("error: invalid lexx [%1] pointer").arg(strLanguageName.c_str()))
                           );
            mb.exec();
            return;
        }
        setILexer( reinterpret_cast<sptr_t>(plex) );
        setReadOnly(true);

        setCodePage(SC_CP_UTF8);
        setProperty("fold", "1"); // show Folders!!
        setProperty("fold.compact", "1");
        // for HTML!!! see C:\Qt\Examples\qt_6\notepad-plus-plus-master\lexilla\lexers\LexHTML.cxx
        setProperty("fold.html", "1"); // "Folding is turned on or off for HTML and XML files with this option. "
        setProperty("fold.html.preprocessor", "1"); //"Folding is turned on or off for scripts embedded in HTML files with this option. "
        setProperty("fold.hypertext.comment", "1"); //"Allow folding for comments in scripts embedded in HTML. "
        setProperty("fold.xml.at.tag.open", "1");   // "Enable folding for XML at the start of open tag. "
        setAutomaticFold(SCI_SETAUTOMATICFOLD);

        setKeyWords(0,R"(
html
head
)");
        const _colors::_color c_xz           {_colors::teal};
        setStyleHlp( SCE_H_DEFAULT,          _colors::black );
        setStyleHlp( SCE_H_TAG,              _colors::blue, true );
        setStyleHlp( SCE_H_TAGUNKNOWN,       _colors::blue, true );
        setStyleHlp( SCE_H_ATTRIBUTE,        _colors::blue );
        setStyleHlp( SCE_H_ATTRIBUTEUNKNOWN, _colors::blue );
        setStyleHlp( SCE_H_NUMBER,           _colors::red );
        setStyleHlp( SCE_H_DOUBLESTRING ,    _colors::fuchsia );
        setStyleHlp( SCE_H_SINGLESTRING ,    _colors::aqua );
        setStyleHlp( SCE_H_OTHER ,           _colors::aqua );
        setStyleHlp( SCE_H_COMMENT ,         _colors::green );
        setStyleHlp( SCE_H_ENTITY ,          _colors::olive );
        setStyleHlp( SCE_H_TAGEND ,          _colors::blue, true );
        setStyleHlp( SCE_H_XMLSTART ,        _colors::navy );
        setStyleHlp( SCE_H_XMLEND ,          _colors::olive );
        setStyleHlp( SCE_H_SCRIPT ,          _colors::yellow );
        setStyleHlp( SCE_H_ASP , c_xz );
        setStyleHlp( SCE_H_ASPAT , c_xz );
        setStyleHlp( SCE_H_CDATA ,           _colors::navy );
        setStyleHlp( SCE_H_QUESTION , c_xz );
        setStyleHlp( SCE_H_VALUE , c_xz );
        setStyleHlp( SCE_H_XCCOMMENT ,       _colors::green );
        setStyleHlp( SCE_H_SGML_DEFAULT , c_xz );
        setStyleHlp( SCE_H_SGML_COMMAND , c_xz );
        setStyleHlp( SCE_H_SGML_1ST_PARAM , c_xz );
        setStyleHlp( SCE_H_SGML_DOUBLESTRING , c_xz );
        setStyleHlp( SCE_H_SGML_SIMPLESTRING, c_xz );
        setStyleHlp( SCE_H_SGML_ERROR , c_xz );
        setStyleHlp( SCE_H_SGML_SPECIAL , c_xz );
        setStyleHlp( SCE_H_SGML_ENTITY , c_xz );
        setStyleHlp( SCE_H_SGML_COMMENT , c_xz );
        setStyleHlp( SCE_H_SGML_1ST_PARAM_COMMENT , c_xz );
        setStyleHlp( SCE_H_SGML_BLOCK_DEFAULT , c_xz );
        setStyleHlp( SCE_HJ_START , c_xz );
        setStyleHlp( SCE_HJ_DEFAULT , c_xz );
        setStyleHlp( SCE_HJ_COMMENT , c_xz );
        setStyleHlp( SCE_HJ_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HJ_COMMENTDOC , c_xz );
        setStyleHlp( SCE_HJ_NUMBER , c_xz );
        setStyleHlp( SCE_HJ_WORD , c_xz );
        setStyleHlp( SCE_HJ_KEYWORD , c_xz );
        setStyleHlp( SCE_HJ_DOUBLESTRING , c_xz );
        setStyleHlp( SCE_HJ_SINGLESTRING , c_xz );
        setStyleHlp( SCE_HJ_SYMBOLS , c_xz );
        setStyleHlp( SCE_HJ_STRINGEOL , c_xz );
        setStyleHlp( SCE_HJ_REGEX , c_xz );
        setStyleHlp( SCE_HJA_START , c_xz );
        setStyleHlp( SCE_HJA_DEFAULT , c_xz );
        setStyleHlp( SCE_HJA_COMMENT , c_xz );
        setStyleHlp( SCE_HJA_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HJA_COMMENTDOC , c_xz );
        setStyleHlp( SCE_HJA_NUMBER , c_xz );
        setStyleHlp( SCE_HJA_WORD , c_xz );
        setStyleHlp( SCE_HJA_KEYWORD , c_xz );
        setStyleHlp( SCE_HJA_DOUBLESTRING , c_xz );
        setStyleHlp( SCE_HJA_SINGLESTRING , c_xz );
        setStyleHlp( SCE_HJA_SYMBOLS , c_xz );
        setStyleHlp( SCE_HJA_STRINGEOL , c_xz );
        setStyleHlp( SCE_HJA_REGEX , c_xz );
        setStyleHlp( SCE_HB_START , c_xz );
        setStyleHlp( SCE_HB_DEFAULT , c_xz );
        setStyleHlp( SCE_HB_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HB_NUMBER , c_xz );
        setStyleHlp( SCE_HB_WORD , c_xz );
        setStyleHlp( SCE_HB_STRING , c_xz );
        setStyleHlp( SCE_HB_IDENTIFIER , c_xz );
        setStyleHlp( SCE_HB_STRINGEOL , c_xz );
        setStyleHlp( SCE_HBA_START , c_xz );
        setStyleHlp( SCE_HBA_DEFAULT , c_xz );
        setStyleHlp( SCE_HBA_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HBA_NUMBER , c_xz );
        setStyleHlp( SCE_HBA_WORD , c_xz );
        setStyleHlp( SCE_HBA_STRING , c_xz );
        setStyleHlp( SCE_HBA_IDENTIFIER , c_xz );
        setStyleHlp( SCE_HBA_STRINGEOL , c_xz );
        setStyleHlp( SCE_HP_START , c_xz );
        setStyleHlp( SCE_HP_DEFAULT , c_xz );
        setStyleHlp( SCE_HP_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HP_NUMBER , c_xz );
        setStyleHlp( SCE_HP_STRING , c_xz );
        setStyleHlp( SCE_HP_CHARACTER , c_xz );
        setStyleHlp( SCE_HP_WORD , c_xz );
        setStyleHlp( SCE_HP_TRIPLE , c_xz );
        setStyleHlp( SCE_HP_TRIPLEDOUBLE , c_xz );
        setStyleHlp( SCE_HP_CLASSNAME , c_xz );
        setStyleHlp( SCE_HP_DEFNAME , c_xz );
        setStyleHlp( SCE_HP_OPERATOR , c_xz );
        setStyleHlp( SCE_HP_IDENTIFIER , c_xz );
        setStyleHlp( SCE_HPHP_COMPLEX_VARIABLE , c_xz );
        setStyleHlp( SCE_HPA_START , c_xz );
        setStyleHlp( SCE_HPA_DEFAULT , c_xz );
        setStyleHlp( SCE_HPA_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HPA_NUMBER , c_xz );
        setStyleHlp( SCE_HPA_STRING , c_xz );
        setStyleHlp( SCE_HPA_CHARACTER , c_xz );
        setStyleHlp( SCE_HPA_WORD , c_xz );
        setStyleHlp( SCE_HPA_TRIPLE , c_xz );
        setStyleHlp( SCE_HPA_TRIPLEDOUBLE , c_xz );
        setStyleHlp( SCE_HPA_CLASSNAME , c_xz );
        setStyleHlp( SCE_HPA_DEFNAME , c_xz );
        setStyleHlp( SCE_HPA_OPERATOR , c_xz );
        setStyleHlp( SCE_HPA_IDENTIFIER , c_xz );
        setStyleHlp( SCE_HPHP_DEFAULT , c_xz );
        setStyleHlp( SCE_HPHP_HSTRING , c_xz );
        setStyleHlp( SCE_HPHP_SIMPLESTRING , c_xz );
        setStyleHlp( SCE_HPHP_WORD , c_xz );
        setStyleHlp( SCE_HPHP_NUMBER , c_xz );
        setStyleHlp( SCE_HPHP_VARIABLE , c_xz );
        setStyleHlp( SCE_HPHP_COMMENT , c_xz );
        setStyleHlp( SCE_HPHP_COMMENTLINE , c_xz );
        setStyleHlp( SCE_HPHP_HSTRING_VARIABLE , c_xz );
        setStyleHlp( SCE_HPHP_OPERATOR , c_xz );
    }
}
const char* SciHlp::MonospaceFont(){
    static char fontNameDefault[200] = "";
    if (!fontNameDefault[0]) {
        const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        strcpy(fontNameDefault, font.family().toUtf8());
    }
    return fontNameDefault;
}
void SciHlp::tstSci(){
#if _WIN32 //https://www.scintilla.org/LexillaDoc.html
    typedef int ( __stdcall* _pfGetLexerCount)(void);
    const QFunctionPointer pfGetLexerCount = QLibrary::resolve("lexilla5", "GetLexerCount");
    //EXPORT_FUNCTION const char * CALLING_CONVENTION LexerNameFromID(int identifier) {
    typedef const char* ( __stdcall *_pfLexerNameFromID)(int);
    const QFunctionPointer pfLexerNameFromID = QLibrary::resolve("lexilla5", "LexerNameFromID");
#else
    //typedef void *(*CreateLexerFn)(const char *name);
    //const QFunctionPointer pfn = QLibrary::resolve("libLexilla.so.5", "CreateLexer");
#endif
#if _WIN32
    const int nNumLexs = ( reinterpret_cast<_pfGetLexerCount>(pfGetLexerCount) )(  );
    for( int nLexsNum = 0 ; nLexsNum < nNumLexs ; nLexsNum++ ){
        const char*  pchLexName = ( reinterpret_cast<_pfLexerNameFromID>(pfLexerNameFromID) )( nLexsNum );
        if(pchLexName)
            qDebug()<<"pchLexName=["<<nLexsNum<<"]= "<<pchLexName;
        else
            qDebug()<<"pchLexName=["<<nLexsNum<<"]= MY_NULL!!";
    }
#endif
}
void SciHlp::showEvent(QShowEvent *event){
}
void SciHlp::onMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin) {
    if(margin == 1) {
        toggleFold(lineFromPosition(position));
    }
}
void SciHlp::onNotify(Scintilla::NotificationData* pnd){
    switch (pnd->nmhdr.code) {
        case Scintilla::Notification::CharAdded:   { // I try https://www.scintilla.org/ScintillaUsage.html - "Implementing Auto-Indent" - but it's not work properly and I remake:
            if( (pnd->ch == '\r') || (pnd->ch == '\n') ) {
                char linebuf[1000];
                const sptr_t n_curr_pos  = currentPos();
                const sptr_t n_curr_line = lineFromPosition(n_curr_pos);
                const sptr_t n_line_len  = lineLength(n_curr_line);
                if( (n_curr_line > 0) && (n_line_len <= 2) )  {
                    const sptr_t n_prev_line_len = lineLength(n_curr_line-1);
                    if(n_prev_line_len < sizeof(linebuf)){
                        const std::size_t buflen = sizeof(linebuf);
                        QByteArray qbaLinePrev = getLine(n_curr_line-1);
                        QByteArray::iterator iter_qbaLinePrev;
                        int pos = 0;
                        for(iter_qbaLinePrev = qbaLinePrev.begin() ; iter_qbaLinePrev != qbaLinePrev.end() ; iter_qbaLinePrev++ ){
                            assert((*iter_qbaLinePrev) != '\t');
                            if( (*iter_qbaLinePrev) == ' ' ){
                                linebuf[pos] = ' ';
                                pos++;
                            } else {
                                break;
                            }
                        }
                        linebuf[pos] = '\0';
                        setSelection(n_curr_pos,n_curr_pos);
                        replaceSel(linebuf);
                        //sptr_t spTxtLen = textLength();
                        //QByteArray qbaTxt =  getText(spTxtLen);
                        //qDebug("\n");
                    }
                }
            }
        }//case Scintilla::Notification::CharAdded:
        break;
    }
}
void SciHlp::setStyleHlp(sptr_t style, sptr_t fore, bool bold, bool italic, sptr_t back, bool underline, bool eolfilled){
    styleSetFore      ( style, fore      );
    styleSetBold      ( style, bold      );
    styleSetItalic    ( style, italic    );
    styleSetBack      ( style, back      );
    styleSetUnderline ( style, underline );
    styleSetEOLFilled ( style, eolfilled );
}
SciHlp::_ret_vals SciHlp::setContent(const std::string& str_text){
    bool bl_read_only_prev = false;
    if(canPaste() == false){
        bl_read_only_prev = true;
        setReadOnly(false);
    }
    setText(str_text.c_str());
    //emptyUndoBuffer();
    setSavePoint();
    if(bl_read_only_prev == true)
        setReadOnly(true);
    return _ret_vals::ok;
}
SciHlp::_ret_vals SciHlp::my_appendTect(const std::string_view svTxt){
    bool bl_read_only_prev = false;
    if(canPaste() == false){
        bl_read_only_prev = true;
        setReadOnly(false);
    }
    //setText(str_text.c_str());
    appendText(svTxt.length(),svTxt.data());
    if(bl_read_only_prev == true)
        setReadOnly(true);
    return _ret_vals::ok;
}
bool SciHlp::getContentModified() const {
    return modify();
}
SciHlp::_ret_vals SciHlp::setFileInfo(const QFileInfo& fiNew ){
    fiFileSource_ = fiNew;
    emit chngFileInfo(fiNew);
    return _ret_vals::ok;
}
const QFileInfo& SciHlp::getFileInfo() const {
    return fiFileSource_;
}
SciHlp::_ret_vals SciHlp::ContentToFile(){
    try{
        QFile qFile{ fiFileSource_.absoluteFilePath() };
        if(qFile.open(QIODevice::WriteOnly)==false){
            return _ret_vals::failure;
        }
        QTextStream tsFile{&qFile};
        QByteArray baContent { getText(textLength()+1) };
        tsFile << QString::fromUtf8(baContent);
        tsFile.flush();
        qFile.close();
        setSavePoint();
    }catch(...){
        return _ret_vals::failure;
    }
    return _ret_vals::ok;
}
SciHlp::_ret_vals SciHlp::ContentFromFile(){
    try{
        QFile qFile{ fiFileSource_.absoluteFilePath() };
        if(qFile.open(QIODevice::ReadOnly|QIODevice::Text)==false){
            return _ret_vals::failure;
        }
        QString qstrFileContent{ qFile.readAll() };
        std::string str{ qstrFileContent.toStdString() };
        setText(str.c_str()); // or
        //setText(qstrFileContent.toLocal8Bit().data()); // https://wiki.qt.io/Technical_FAQ#How_can_I_convert_a_QString_to_char.2A_and_vice_versa.3F // не работает с русскими буквами
        qFile.close();
        setSavePoint();
    }catch(...){
        return _ret_vals::failure;
    }
    return _ret_vals::ok;
}
SciHlp::_ret_vals SciHlp::Find(_params_find params_find){
    //SCFIND_NONE	    Default setting is case-insensitive literal match.
    //SCFIND_MATCHCASE	A match only occurs with text that matches the case of the search string.
    //SCFIND_WHOLEWORD	A match only occurs if the characters before and after are not word characters as defined by SCI_SETWORDCHARS.
    //SCFIND_WORDSTART	A match only occurs if the character before is not a word character as defined by SCI_SETWORDCHARS.
    //SCFIND_REGEXP	    The search string should be interpreted as a regular expression. Uses Scintilla's base implementation unless combined with SCFIND_CXX11REGEX.
    //SCFIND_POSIX	    Treat regular expression in a more POSIX compatible manner by interpreting bare ( and ) for tagged sections rather than \( and \). Has no effect when SCFIND_CXX11REGEX is set.
    //SCFIND_CXX11REGEX	This flag may be set to use C++11 <regex> instead of Scintilla's basic regular expressions. If the regular expression is invalid then -1 is returned and status is set to SC_STATUS_WARN_REGEX. The ECMAScript flag is set on the regex object and UTF-8 documents will exhibit Unicode-compliant behaviour. For MSVC, where wchar_t is 16-bits, the regular expression ".." will match a single astral-plane character. There may be other differences between compilers. Must also have SCFIND_REGEXP set.
    const sptr_t n_flags {SCFIND_REGEXP};
    setSearchFlags(n_flags);
    const sptr_t n_pos_curr { currentPos()} ;
    const sptr_t n_pos_max  { length()} ;
    setTargetStart( n_pos_curr );
    setTargetEnd  ( n_pos_max  );
    const QByteArray qaFind = params_find.qstrFind_.toUtf8();
    const sptr_t n_pos_find = searchInTarget( qaFind.length(), qaFind );
    if(-1 != n_pos_find){
        gotoPos    ( targetStart() );
        setSel     ( targetStart(), targetEnd() );
        scrollRange( targetStart(), targetEnd() );
        return _ret_vals::ok;
    }
    return _ret_vals::failure;
}
