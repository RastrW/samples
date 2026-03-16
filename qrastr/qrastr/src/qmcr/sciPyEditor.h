#pragma once

#include <QFileInfo>
#include "sciHlpBase.h"

/// Редактор Python-макросов: read-write, подсветка Python, автоотступ, фолдинг.
class SciPyEditor : public SciHlpBase
{
    Q_OBJECT
public:
    struct FindParams {
        explicit FindParams(const QString& text) : m_text(text) {}
        QString m_text;
        bool    wrapAround = true;
    };

    explicit SciPyEditor(QWidget* parent);
    ~SciPyEditor() override = default;

    bool   isModified() const;

    RetVal setFileInfo(const QFileInfo& fi);
    const QFileInfo& getFileInfo() const;

    RetVal loadFromFile();
    RetVal saveToFile();
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
    RetVal find(const FindParams& params);
    void   gotoLine(sptr_t zeroBasedLine);

signals:
    void sig_fileInfoChanged(const QFileInfo& fi);

private slots:
    void slot_marginClicked(Scintilla::Position pos,
                            Scintilla::KeyMod   modifiers,
                            int                 margin);
    void slot_notify(Scintilla::NotificationData* pnd);

private:
    void setupPythonLexer();

    QFileInfo m_fileInfo;
};
