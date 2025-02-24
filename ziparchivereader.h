#ifndef ZIPARCHIVEREADER_H
#define ZIPARCHIVEREADER_H
#include "archivereader.h"

class ZipArchiveReader:public ArchiveReader
{
public:
    ZipArchiveReader();

    // ArchiveReader interface
public:
    QStringList pageList(const QString &path, const QSet<QString> &suffixes) override;
    QPixmap pageImage(const QString& bookPath, const QString &pagePath) override;
    bool supportArchive(const QString &path) override;

    // ArchiveReader interface
public:
    QString archiveType() override;
};

#endif // ZIPARCHIVEREADER_H
