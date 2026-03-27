#include <QLibrary>
#include <QMessageBox>
#include <QFontDatabase>
#include <QTextStream>
#include "sciHlpBase.h"
#include <spdlog/spdlog.h>

SciHlpBase::SciHlpBase(QWidget* parent)
    : ScintillaEdit(parent){

    //By default,
    //margin 0 is set to display line numbers, but is given a width of 0, so it is hidden.
    //Margin 1 is set to display non-folding symbols and is given a width of 16 pixels, so it is visible.
    //Margin 2 is set to display the folding symbols, but is given a width of 0, so it is hidden.
    //Of course, you can set the margins to be whatever you wish.
    //https://www.scintilla.org/ScintillaDoc.html#SCI_GETMARGINTYPEN
    //https://stackoverflow.com/questions/78522506/scintilla-will-not-highlight-or-codefold-in-my-c
    //https://www.purebasic.fr/english/viewtopic.php?t=68691
    //https://github.com/jacobslusser/ScintillaNET/issues/307
    styleSetFont(STYLE_DEFAULT, monospaceFontName());
    setupMargins();
    setupFolding();
    setUseTabs(false);
    setTabIndents(true);
    setTabWidth(4);
    setWrapIndentMode(SC_WRAPINDENT_INDENT);
    markerSetBack(0,1);
    markerSetBack(1,1);
}

void SciHlpBase::setupMargins()
{
    setMarginWidthN(k_marginLineNum, 40);
    setMarginWidthN(k_marginFold,    20);
    setMarginMaskN (k_marginFold, SC_MASK_FOLDERS);
    setMarginTypeN (k_marginFold, SC_MARGIN_SYMBOL);
    setMarginSensitiveN(k_marginFold, true);
}

void SciHlpBase::setupFolding()
{
    markerDefine(SC_MARKNUM_FOLDER,        SC_MARK_BOXPLUS);
    markerDefine(SC_MARKNUM_FOLDEROPEN,    SC_MARK_BOXMINUS);
    markerDefine(SC_MARKNUM_FOLDERSUB,     SC_MARK_VLINE);
    markerDefine(SC_MARKNUM_FOLDERTAIL,    SC_MARK_LCORNER);
    markerDefine(SC_MARKNUM_FOLDEREND,     SC_MARK_BOXPLUSCONNECTED);
    markerDefine(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    markerDefine(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
}

void SciHlpBase::setStyleHlp(sptr_t style,
                             _colors::color_t fore,
                             bool bold, bool italic,
                             _colors::color_t back,
                             bool underline, bool eolfilled)
{
    styleSetFore     (style, static_cast<sptr_t>(fore));
    styleSetBold     (style, bold);
    styleSetItalic   (style, italic);
    styleSetBack     (style, static_cast<sptr_t>(back));
    styleSetUnderline(style, underline);
    styleSetEOLFilled(style, eolfilled);
}

SciHlpBase::CreateLexerFn SciHlpBase::resolveLexerFactory()
{
#ifdef _WIN32
    const QFunctionPointer pfn = QLibrary::resolve("lexilla5", "CreateLexer");
#else
    const QFunctionPointer pfn = QLibrary::resolve("liblexilla", "CreateLexer");
#endif
    if (!pfn) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("Cannot resolve lexilla.CreateLexer"));
    }
    return reinterpret_cast<CreateLexerFn>(pfn);
}

const char* SciHlpBase::monospaceFontName()
{
    static std::string s_name;
    if (s_name.empty())
        s_name = QFontDatabase::systemFont(QFontDatabase::FixedFont)
                     .family().toStdString();
    return s_name.c_str();
}

void SciHlpBase::showEvent(QShowEvent* event)
{
    ScintillaEdit::showEvent(event);
}

SciHlpBase::RetVal SciHlpBase::setContent(const std::string& text)
{
    const bool wasRo = !canPaste();
    if (wasRo) setReadOnly(false);
    setText(text.c_str());
    setSavePoint();
    if (wasRo) setReadOnly(true);
    return RetVal::Ok;
}

SciHlpBase::RetVal SciHlpBase::appendTextCustom(std::string_view sv)
{
    const bool wasRo = !canPaste();
    if (wasRo) setReadOnly(false);
    appendText(static_cast<sptr_t>(sv.size()), sv.data());
    if (wasRo) setReadOnly(true);
    return RetVal::Ok;
}

void SciHlpBase::showAllLexer(){
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
            spdlog::debug("pchLexName = [{}]= {}", nLexsNum, pchLexName);
        else
            spdlog::debug("pchLexName=[{}]= MY_NULL!!", nLexsNum);
    }
#endif
}
