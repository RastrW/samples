#include "recentFilesManager.h"
#include <spdlog/spdlog.h>
#include <QSettings>
#include "settingsKeys.h"

void RecentFilesManager::add(const QString& filePath, const QString& templatePath) {
    QSettings s;
    const int maxRecent = s.value(SK::Files::maxRecentFiles,
                                  SK::Files::defMaxRecent).toInt();
    auto entries = load();
    // Убираем дубликат по пути файла
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [&](const RecentFileEntry& e) { return e.file == filePath; }),
        entries.end());

    entries.prepend({filePath, templatePath});

    while (entries.size() > maxRecent)
        entries.removeLast();

    save(entries);
}

QList<RecentFileEntry> RecentFilesManager::load() const {
    QSettings s;
    const int n = s.beginReadArray(SK::Files::recentFiles);
    QList<RecentFileEntry> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        result.append({s.value("file").toString(),
                       s.value("tmpl").toString()});
    }
    s.endArray();
    return result;
}

void RecentFilesManager::save(const QList<RecentFileEntry>& entries) {
    QSettings s;
    s.remove(SK::Files::recentFiles);
    s.beginWriteArray(SK::Files::recentFiles);
    for (int i = 0; i < entries.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("file", entries[i].file);
        s.setValue("tmpl", entries[i].tmpl);
    }
    s.endArray();
}