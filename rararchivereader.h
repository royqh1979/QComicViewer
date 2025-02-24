#ifndef RARARCHIVEREADER_H
#define RARARCHIVEREADER_H
#include "archivereader.h"

class RarArchiveReader : public ArchiveReader
{
public:
    RarArchiveReader();

    // ArchiveReader interface
public:
    QStringList pageList(const QString &path, const QSet<QString> &suffixes) override;
    QPixmap pageImage(const QString &bookPath, const QString &pagePath) override;
    bool supportArchive(const QString &path) override;

    // ArchiveReader interface
public:
    QString archiveType() override;
};

#endif // RARARCHIVEREADER_H
