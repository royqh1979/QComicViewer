#include "rararchivereader.h"
#include "qtrar/qtrar.h"
#include "qtrar/qtrarfile.h"

#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>

RarArchiveReader::RarArchiveReader()
{

}

QStringList RarArchiveReader::pageList(const QString &path, const QSet<QString> &suffixes)
{
    QStringList result;
    QtRAR archive{path};
    if (archive.open(QtRAR::OpenModeList)) {
        QStringList fileList = archive.fileNameList();
        foreach(const QString &file, fileList) {
            QFileInfo info{file};
            QString suffix = info.suffix().toLower();
            if (suffixes.contains(suffix))
                result.append(file);
        }
        archive.close();
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

QPixmap RarArchiveReader::pageImage(const QString &bookPath, const QString &pagePath)
{
    QtRAR archive{bookPath};
    if (!archive.open(QtRAR::OpenModeExtract))
        return QPixmap();
    QtRARFile unrarFile{&archive};
    unrarFile.setFileName(pagePath);
    if (!unrarFile.open(QIODevice::ReadOnly))
        return QPixmap();
    QByteArray contents = unrarFile.readAll();
    QBuffer buffer{&contents};
    QImageReader reader{&buffer};
    QPixmap result = QPixmap::fromImageReader(&reader);
    unrarFile.close();
    archive.close();
    return result;
}

bool RarArchiveReader::supportArchive(const QString &path)
{
    QFileInfo info{path};
    return info.exists() && info.isFile() && (path.endsWith(".rar", Qt::CaseInsensitive)
                                              || path.endsWith(".cbr", Qt::CaseInsensitive));
}

QString RarArchiveReader::archiveType()
{
    return "rar";
}
