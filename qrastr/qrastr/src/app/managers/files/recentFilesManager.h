#pragma once
#include <QList>

struct RecentFileEntry {
    QString file;
    QString tmpl;  // может быть пустым
};

///@class Менеджер недавних файлов
class RecentFilesManager {
public:
    void add(const QString& filePath, const QString& templatePath);

    [[nodiscard]]
    QList<RecentFileEntry> load() const;

private:
    void save(const QList<RecentFileEntry>& entries);
};