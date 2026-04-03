#pragma once

// Все ключи QSettings в одном месте.
// Использование: QSettings s; s.value(SK::Protocol::collapseCleanStages, true)
//
// Соглашения:
//   - Группа = namespace = секция в реестре/ini-файле
//   - Дефолт хранится рядом с ключом — не разбросан по коду
namespace SK {

namespace Protocol {
    constexpr auto collapseCleanStages = "protocol/collapseCleanStages";
    constexpr auto copyAsXml           = "protocol/copyAsXml";

    // Дефолты рядом с ключами — один источник правды
    constexpr bool defCollapseCleanStages = true;
    constexpr bool defCopyAsXml           = false;
}

namespace MainWindow {
    constexpr auto groupName       = "MainWindow";
    constexpr auto geometry        = "MainWindow/geometry";
    constexpr auto state           = "MainWindow/state";   // toolbar + docks
    constexpr auto appStyle        = "appStyle";

    constexpr auto defAppStyle     = "windows11";
}

namespace Files {
    constexpr auto recentFiles     = "recentFiles";
    constexpr int  defMaxRecent    = 10;
    constexpr auto maxRecentFiles  = "maxRecentFiles";
}

namespace CondFormat {
    constexpr auto font            = "condFormat/font";
    constexpr auto fontSize        = "condFormat/fontsize";
    constexpr auto regFgColour     = "condFormat/reg_fg_colour";

    constexpr int  defFontSize     = 10;
}

namespace Workspaces {
    constexpr auto names           = "Workspaces/names";
    constexpr auto startup         = "Workspaces/startup";
    constexpr auto group           = "Workspaces";
}
} // namespace SK