#ifndef FOLDERARCHIVEREADER_H
#define FOLDERARCHIVEREADER_H

#include "archivereader.h"

class FolderArchiveReader : public ArchiveReader
{
public:
    FolderArchiveReader();

    // ArchiveReader interface
public:
    QStringList pageList(const QString &path, const QSet<QString> &suffixes) override;
    QPixmap pageImage(const QString &bookPath, const QString &pagePath) override;
    bool supportArchive(const QString &path) override;

    // ArchiveReader interface
public:
    QString archiveType() override;
};

#endif // FOLDERARCHIVEREADER_H
