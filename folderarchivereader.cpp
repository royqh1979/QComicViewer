#include "folderarchivereader.h"

#include <QDir>

FolderArchiveReader::FolderArchiveReader()
{

}

QStringList FolderArchiveReader::pageList(const QString &path, const QSet<QString> &suffixes)
{
    QDir dir{path};
    QStringList result;
    QStringList fileList = dir.entryList(QDir::Filter::Files);
    foreach(const QString &file, dir.entryList(QDir::Filter::Files)) {
        QFileInfo info{file};
        QString suffix = info.suffix().toLower();
        if (suffixes.contains(suffix))
            result.append(file);
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

QPixmap FolderArchiveReader::pageImage(const QString &bookPath, const QString &pagePath)
{
    QDir dir{bookPath};
    QString imagePath = dir.absoluteFilePath(pagePath);
    return QPixmap(imagePath);
}

bool FolderArchiveReader::supportArchive(const QString &path)
{
    QFileInfo info{path};
    return info.exists() && info.isDir();
}
