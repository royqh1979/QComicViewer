#include "folderarchivereader.h"

#include <QDir>
#include <QDebug>
#include <QImageReader>

FolderArchiveReader::FolderArchiveReader()
{

}

QStringList FolderArchiveReader::pageList(const QString &path, const QSet<QString> &suffixes)
{
    return pageList(path,"",suffixes);
}

QStringList FolderArchiveReader::pageList(const QString &path, const QString& prefix, const QSet<QString> &suffixes)
{
    QDir dir{path};
    QStringList result;
    QStringList dirs;
    foreach(const QString &subDir, dir.entryList(QDir::Filter::Dirs)) {
        if (subDir.startsWith("."))
            continue;
        dirs+=subDir;
    }
    dirs.sort(Qt::CaseInsensitive);
    foreach(const QString &subDir, dirs) {
        result += pageList(dir.absoluteFilePath(subDir), prefix.isEmpty()?subDir:prefix+"/"+subDir, suffixes);
    }
    QStringList files;
    foreach(const QString &file, dir.entryList(QDir::Filter::Files)) {
        QFileInfo info{file};
        QString suffix = info.suffix().toLower();
        if (suffixes.contains(suffix)) {
            if (prefix.isEmpty())
                files.append(file);
            else
                files.append(prefix+"/"+file);
        }
    }
    files.sort(Qt::CaseInsensitive);
    result+=files;
    return result;
}

QPixmap FolderArchiveReader::pageImage(const QString &bookPath, const QString &pagePath)
{
    QDir dir{bookPath};
    QString imagePath = dir.absoluteFilePath(pagePath);
    QImageReader reader{imagePath};
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull())
        return QPixmap();
    //qDebug()<<pagePath;
    return QPixmap::fromImage(image);
}

bool FolderArchiveReader::supportArchive(const QString &path)
{
    QFileInfo info{path};
    return info.exists() && info.isDir();
}

QString FolderArchiveReader::archiveType()
{
    return "folder";
}
