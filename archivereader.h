#ifndef ARCHIVEREADER_H
#define ARCHIVEREADER_H

#include <QPixmap>
#include <QStringList>
#include <QSet>

class ArchiveReader
{
public:
    ArchiveReader();
    virtual QStringList pageList(const QString& path, const QSet<QString> &suffixes) = 0;
    virtual QPixmap pageImage(const QString& bookPath, const QString& pagePath) = 0;
    virtual bool supportArchive(const QString& path) = 0;
    virtual QString archiveType() = 0;
};

#endif // ARCHIVEREADER_H
