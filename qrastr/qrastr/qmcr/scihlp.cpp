#include <QLibrary>
#include <QMessageBox>
#include <QFontDatabase>
#include <QTextStream>
#include "SciLexer.h"
#include "scihlp.h"

//all of wrapped shit can be found in -> ScintillaEdit.cpp <-
//interesting scintilla use https://github.com/SolarAquarion/wxglterm/tree/master/src/external_plugins
//https://github.com/mneuroth/SciTEQt

const char *MonospaceFont(){
    static char fontNameDefault[200] = "";
    if (!fontNameDefault[0]) {
        const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        strcpy(fontNameDefault, font.family().toUtf8());
    }
    return fontNameDefault;
}
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

    connect(this, SIGNAL(marginClicked( Scintilla::Position, Scintilla::KeyMod, int ) ), this, SLOT(onMarginClicked( Scintilla::Position, Scintilla::KeyMod, int ) ) );
    connect(this, SIGNAL(notify       ( Scintilla::NotificationData*                ) ), this, SLOT(onNotify       ( Scintilla::NotificationData* )                ) );
}
void SciHlp::onMarginClicked(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin) {
    if(margin == 1) {
        toggleFold(lineFromPosition(position));
    }
}
void SciHlp::onNotify(Scintilla::NotificationData* pnd ){
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
    setText(str_text.c_str());
    //emptyUndoBuffer();
    //setSavePoint();

    setFileInfo( QFileInfo(R"(C:\projects\git_web\samples\qrastr\qrastr\qmcr\tst.py)") );
    ContentToFile();

    return _ret_vals::ok;
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
        //setFileInfo(QFileInfo{path2File});
        setSavePoint();
    }catch(...){
        return _ret_vals::failure;
    }
    return _ret_vals::ok;
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
                        QString(tr("error: invalid get function lexilla5.CreateLexer"))
                       );
        mb.exec();
        return;
    }
    const std::string strLanguageName {"python"};
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
    setStyleHlp( SCE_P_TRIPLE ,       _colors::green );        // ''' '''  - multiline comment
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
    setProperty("fold", "1"); // show Folders!!
    setProperty("fold.compact", "0");
    setAutomaticFold(true);
};
