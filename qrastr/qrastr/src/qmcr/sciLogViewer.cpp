#include "sciLogViewer.h"
#include "SciLexer.h"
#include <QMessageBox>

SciLogViewer::SciLogViewer(QWidget* parent)
    : SciHlpBase(parent)
{
    setupXmlLexer();
    setReadOnly(true);
}

void SciLogViewer::setupXmlLexer()
{
    const CreateLexerFn createLexer = resolveLexerFactory();
    if (!createLexer) return;

    const void* plex = createLexer("xml");
    if (!plex) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("Cannot create XML lexer"));
        return;
    }
    setILexer(reinterpret_cast<sptr_t>(plex));
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
    constexpr auto c_xz = _colors::teal;

    setStyleHlp(SCE_H_DEFAULT,          _colors::black       );
    setStyleHlp(SCE_H_TAG,              _colors::blue, true  );
    setStyleHlp(SCE_H_TAGUNKNOWN,       _colors::blue, true  );
    setStyleHlp(SCE_H_ATTRIBUTE,        _colors::blue        );
    setStyleHlp(SCE_H_ATTRIBUTEUNKNOWN, _colors::blue        );
    setStyleHlp(SCE_H_NUMBER,           _colors::red         );
    setStyleHlp(SCE_H_DOUBLESTRING,     _colors::fuchsia     );
    setStyleHlp(SCE_H_SINGLESTRING,     _colors::aqua        );
    setStyleHlp(SCE_H_OTHER,            _colors::aqua        );
    setStyleHlp(SCE_H_COMMENT,          _colors::green       );
    setStyleHlp(SCE_H_ENTITY,           _colors::olive       );
    setStyleHlp(SCE_H_TAGEND,           _colors::blue, true  );
    setStyleHlp(SCE_H_XMLSTART,         _colors::navy        );
    setStyleHlp(SCE_H_XMLEND,           _colors::olive       );
    setStyleHlp(SCE_H_CDATA,            _colors::navy        );
    setStyleHlp(SCE_H_XCCOMMENT ,       _colors::green );

    // Все embedded-стили (JS/VB/PHP/Python в HTML) красим одним цветом
    const std::initializer_list<sptr_t> embeddedStyles = {
        SCE_H_ASP, SCE_H_ASPAT, SCE_H_QUESTION, SCE_H_VALUE,
        SCE_H_SGML_DEFAULT, SCE_H_SGML_COMMAND, SCE_H_SGML_1ST_PARAM,
        SCE_H_SGML_DOUBLESTRING, SCE_H_SGML_SIMPLESTRING, SCE_H_SGML_ERROR, SCE_H_SGML_SPECIAL,
        SCE_H_SGML_ENTITY, SCE_H_SGML_COMMENT, SCE_H_SGML_1ST_PARAM_COMMENT, SCE_H_SGML_BLOCK_DEFAULT,
        SCE_HJ_START, SCE_HJ_DEFAULT, SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC,
        SCE_HJ_NUMBER, SCE_HJ_WORD, SCE_HJ_KEYWORD, SCE_HJ_DOUBLESTRING, SCE_HJ_SINGLESTRING,
        SCE_HJ_SYMBOLS, SCE_HJ_STRINGEOL, SCE_HJ_REGEX, SCE_HJA_START, SCE_HJA_DEFAULT,
        SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, SCE_HJA_NUMBER, SCE_HJA_WORD,
        SCE_HJA_KEYWORD, SCE_HJA_DOUBLESTRING, SCE_HJA_SINGLESTRING, SCE_HJA_SYMBOLS, SCE_HJA_STRINGEOL,
        SCE_HJA_REGEX, SCE_HB_START, SCE_HB_DEFAULT, SCE_HB_COMMENTLINE, SCE_HB_NUMBER, SCE_HB_WORD,
        SCE_HB_STRING, SCE_HB_IDENTIFIER, SCE_HB_STRINGEOL, SCE_HBA_START, SCE_HBA_DEFAULT, SCE_HBA_COMMENTLINE,
        SCE_HBA_NUMBER, SCE_HBA_WORD, SCE_HBA_STRING, SCE_HBA_IDENTIFIER, SCE_HBA_STRINGEOL, SCE_HP_START,
        SCE_HP_DEFAULT, SCE_HP_COMMENTLINE, SCE_HP_NUMBER, SCE_HP_STRING, SCE_HP_CHARACTER, SCE_HP_WORD,
        SCE_HP_TRIPLE, SCE_HP_TRIPLEDOUBLE, SCE_HP_CLASSNAME, SCE_HP_DEFNAME, SCE_HP_OPERATOR,
        SCE_HP_IDENTIFIER, SCE_HPHP_COMPLEX_VARIABLE, SCE_HPA_START, SCE_HPA_DEFAULT, SCE_HPA_COMMENTLINE,
        SCE_HPA_NUMBER, SCE_HPA_STRING, SCE_HPA_CHARACTER, SCE_HPA_WORD, SCE_HPA_TRIPLE, SCE_HPA_TRIPLEDOUBLE,
        SCE_HPA_CLASSNAME, SCE_HPA_DEFNAME, SCE_HPA_OPERATOR, SCE_HPA_IDENTIFIER, SCE_HPHP_DEFAULT, SCE_HPHP_HSTRING,
        SCE_HPHP_SIMPLESTRING, SCE_HPHP_WORD, SCE_HPHP_NUMBER, SCE_HPHP_VARIABLE, SCE_HPHP_COMMENT,
        SCE_HPHP_COMMENTLINE, SCE_HPHP_HSTRING_VARIABLE, SCE_HPHP_OPERATOR
    };
    for (sptr_t s : embeddedStyles)
        setStyleHlp(s, c_xz);
}
