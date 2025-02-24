#include "ziparchivereader.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#include <QBuffer>
#include <QImageReader>

ZipArchiveReader::ZipArchiveReader()
{

}

QStringList ZipArchiveReader::pageList(const QString &path, const QSet<QString> &suffixes)
{
    QStringList result;
    QuaZip zip{path};
    if (zip.open(QuaZip::mdUnzip)) {
        QStringList fileList = zip.getFileNameList();
        foreach(const QString &file, fileList) {
            QFileInfo info{file};
            QString suffix = info.suffix().toLower();
            if (suffixes.contains(suffix))
                result.append(file);
        }
        zip.close();
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

QPixmap ZipArchiveReader::pageImage(const QString& bookPath, const QString &pagePath)
{
    QuaZip zip{bookPath};
    if (!zip.open(QuaZip::mdUnzip))
        return QPixmap();
    if (!zip.setCurrentFile(pagePath))
        return QPixmap();
    QuaZipFile unzipFile(&zip);
    if (!unzipFile.open(QIODevice::ReadOnly))
        return QPixmap();
    QByteArray contents = unzipFile.readAll();
    QBuffer buffer{&contents};
    QImageReader reader{&buffer};
    QPixmap result = QPixmap::fromImageReader(&reader);
    unzipFile.close();
    zip.close();
    return result;
}

bool ZipArchiveReader::supportArchive(const QString &path)
{
    QFileInfo info{path};
    return info.exists() && info.isFile() && (path.endsWith(".zip", Qt::CaseInsensitive)
                                              || path.endsWith(".cbz", Qt::CaseInsensitive));
}

QString ZipArchiveReader::archiveType()
{
    return "zip";
}
