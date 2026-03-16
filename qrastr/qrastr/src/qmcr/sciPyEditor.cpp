#include <QLibrary>
#include <QMessageBox>
#include <QFontDatabase>
#include <QTextStream>
#include "SciLexer.h"
#include "sciPyEditor.h"

SciPyEditor::SciPyEditor(QWidget* parent)
    : SciHlpBase(parent)
{
    setupPythonLexer();

    connect(this, &ScintillaEdit::marginClicked,
            this, &SciPyEditor::slot_marginClicked);
    connect(this, &ScintillaEdit::notify,
            this, &SciPyEditor::slot_notify);
}

void SciPyEditor::setupPythonLexer()
{
    const CreateLexerFn createLexer = resolveLexerFactory();
    if (!createLexer) return;

    const void* plex = createLexer("python");
    if (!plex) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("Cannot create Python lexer"));
        return;
    }
    setILexer(reinterpret_cast<sptr_t>(plex));
    setCodePage(SC_CP_UTF8);

    setProperty("fold",               "1"); // show Folders!!
    setProperty("fold.compact",       "0");
    setProperty("fold.quotes.python", "1");
    setAutomaticFold(SCI_SETAUTOMATICFOLD);

    setKeyWords(0, R"(
        False None True and as assert break class continue def del elif else
        except finally for from global if import in is lambda nonlocal not
        or pass raise return try while with yield
        abs aiter all anext any ascii bin bool breakpoint
        bytearray bytes callable chr classmethod compile complex
        delattr dict dir divmod enumerate eval exec filter float format
        frozenset getattr globals hasattr hash help hex id input int
        isinstance issubclass iter len list locals map max memoryview
        min next object oct open ord pow print property range repr
        reversed round set setattr slice sorted staticmethod str sum
        super tuple type vars zip _ __import__
    )");

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
}

void SciPyEditor::slot_marginClicked(Scintilla::Position position,
                                Scintilla::KeyMod modifiers, int margin) {
    if(margin == k_marginFold) {
        toggleFold(lineFromPosition(position));
    }
}

void SciPyEditor::slot_notify(Scintilla::NotificationData* pnd){
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

bool SciPyEditor::isModified() const{
    return modify();
}

SciHlpBase::RetVal SciPyEditor::setFileInfo(const QFileInfo& fi){
    m_fileInfo = fi;
    emit sig_fileInfoChanged(fi);
    return RetVal::Ok;
}

const QFileInfo& SciPyEditor::getFileInfo() const{
    return m_fileInfo;
}

SciHlpBase::RetVal SciPyEditor::loadFromFile()
{
    QFile file(m_fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return RetVal::Failure;

    const QByteArray bytes = file.readAll();
    setText(bytes.constData());
    file.close();
    setSavePoint();
    return RetVal::Ok;
}

SciHlpBase::RetVal SciPyEditor::saveToFile()
{
    QFile file(m_fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return RetVal::Failure;

    const QByteArray content = getText(textLength());
    if (file.write(content) == -1)
        return RetVal::Failure;
    file.close();
    setSavePoint();
    return RetVal::Ok;
}

SciHlpBase::RetVal SciPyEditor::find(const FindParams& params)
{
    setSearchFlags(SCFIND_REGEXP);
    const QByteArray needle = params.m_text.toUtf8();

    // Ищем от текущей позиции до конца
    setTargetStart(currentPos());
    setTargetEnd(length());
    sptr_t found = searchInTarget(needle.size(), needle.constData());

    // Если не нашли и разрешён wrap-around — ищем с начала до текущей позиции
    if (found == -1 && params.wrapAround) {
        setTargetStart(0);
        setTargetEnd(currentPos());
        found = searchInTarget(needle.size(), needle.constData());
    }

    if (found == -1)
        return RetVal::Failure;

    gotoPos    (targetStart());
    setSel     (targetStart(), targetEnd());
    scrollRange(targetStart(), targetEnd());
    return RetVal::Ok;
}

void SciPyEditor::gotoLine(sptr_t zeroBasedLine)
{
    ScintillaEdit::gotoLine(zeroBasedLine);
}
