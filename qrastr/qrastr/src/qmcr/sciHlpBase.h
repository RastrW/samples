#pragma once

#include <QFileInfo>
#include "ScintillaEdit.h"

/// Общая база для редактора и лог-вьювера.
/// Хранит поля, шрифт, цвета, вспомогательные методы.
/// Файловый API и специфика лексера — в наследниках.
/// @note all of wrapped shit can be found in -> ScintillaEdit.cpp <-
/// https://www.scintilla.org/PaneAPI.html
/// interesting scintilla use https://github.com/SolarAquarion/wxglterm/tree/master/src/external_plugins
/// https://github.com/mneuroth/SciTEQt
class SciHlpBase
    : public ScintillaEdit {
    Q_OBJECT
public:
    enum class RetVal { Ok = 1, Failure = -1 };

    SciHlpBase(QWidget *parent);
    ~SciHlpBase() override = default;

    RetVal setContent(const std::string& str_text);
    RetVal appendTextCustom(const std::string_view svTxt);

    void showEvent(QShowEvent *event) override;
protected:
    struct _colors {
        using color_t = unsigned long;

        static constexpr auto make = [](std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) constexpr {
            return static_cast<color_t>(r)
            | (static_cast<color_t>(g) << 8)
                | (static_cast<color_t>(b) << 16)
                    | (static_cast<color_t>(a) << 24);
        };

        static constexpr color_t aqua    = make(0x00, 0xff, 0xff, 0x00 );
        static constexpr color_t black   = make(0x00, 0x00, 0x00, 0x00 );
        static constexpr color_t blue    = make(0x00, 0x00, 0xff, 0x00 );
        static constexpr color_t fuchsia = make(0xff, 0x00, 0xff, 0x00 );
        static constexpr color_t green   = make(0x00, 0x80, 0x00, 0x00 );
        static constexpr color_t gray    = make(0x80, 0x80, 0x80, 0x00 );
        static constexpr color_t lime    = make(0x00, 0xff, 0x00, 0x00 );
        static constexpr color_t maroon  = make(0x80, 0x00, 0x00, 0x00 );
        static constexpr color_t navy    = make(0x00, 0x00, 0x80, 0x00 );
        static constexpr color_t olive   = make(0x80, 0x80, 0x00, 0x00 );
        static constexpr color_t purple  = make(0x80, 0x00, 0x80, 0x00 );
        static constexpr color_t red     = make(0xff, 0x00, 0x00, 0x00 );
        static constexpr color_t silver  = make(0xc0, 0xc0, 0x00, 0x00 );
        static constexpr color_t teal    = make(0x00, 0x80, 0x80, 0x00 );
        static constexpr color_t white   = make(0xff, 0xff, 0xff, 0x00 );
        static constexpr color_t yellow  = make(0xff, 0xff, 0x00, 0x00 );
    };
    void showAllLexer();
    void setupMargins();
    void setupFolding();

    void setStyleHlp(sptr_t style,
                     _colors::color_t fore,
                     bool bold      = false,
                     bool italic    = false,
                     _colors::color_t back = _colors::white,
                     bool underline = false,
                     bool eolfilled = false);

    // Возвращает указатель на фабричную функцию CreateLexer из Lexilla.
    // nullptr если библиотека не найдена.
    using CreateLexerFn = void*(*)(const char*);
    static CreateLexerFn resolveLexerFactory();

    static const char* monospaceFontName();

    static constexpr sptr_t k_marginLineNum = 0;
    static constexpr sptr_t k_marginFold    = 1;
};
