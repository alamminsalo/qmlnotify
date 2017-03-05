#ifndef NOTIFICATIONUTILS_H
#define NOTIFICATIONUTILS_H

#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QIcon>
#include <QDBusArgument>
#include <QDebug>
#include <QDir>

// Static collection of misc utils
namespace utils {

    QByteArray imageToBase64(const QImage &img)
    {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        img.save(&buffer, "PNG"); // writes the image in PNG format inside the buffer

        return "data:image/png;base64," + byteArray.toBase64();
    }

    // Taken from <http://doc.qt.io/qt-5/qtwidgets-dialogs-findfiles-example.html>
    void findRecursion(const QString &path, const QString &pattern, QStringList *result)
    {
        QDir currentDir(path);
        const QString prefix = path + QLatin1Char('/');
        foreach (const QString &match, currentDir.entryList(QStringList(pattern), QDir::Files | QDir::NoSymLinks))
            result->append(prefix + match);
        foreach (const QString &dir, currentDir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot))
            findRecursion(prefix + dir, pattern, result);
    }

    QString findApplicationIconPath(QString appname) {
        QString path = "";

        QString currentTheme = QIcon::themeName();
        QString searchStr = appname.toLower();

        qDebug() << "Finding icon for" << appname <<  "from theme" << currentTheme;

        foreach (QString path, QIcon::themeSearchPaths()) {
            QDir dir(path);
            if (dir.cd(currentTheme)) {
                QStringList matches;

                findRecursion(dir.absolutePath(), appname, &matches);

                if (!matches.isEmpty())
                    qDebug() << matches;
            }
        }

        return path;
    }
}

/**
 * Automatic marshaling of a QImage for org.freedesktop.Notifications.Notify
 *
 * This function is from the Clementine project (see
 * http://www.clementine-player.org) and licensed under the GNU General Public
 * License, version 3 or later.
 *
 * Copyright 2010, David Sansome <me@davidsansome.com>
 */
QDBusArgument& operator<<(QDBusArgument& arg, const QImage& image) {
    if (image.isNull()) {
        // Sometimes this gets called with a null QImage for no obvious reason.
        arg.beginStructure();
        arg << 0 << 0 << 0 << false << 0 << 0 << QByteArray();
        arg.endStructure();
        return arg;
    }
    QImage scaled = image.scaledToHeight(128, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    // ABGR -> ARGB
    QImage i = scaled.rgbSwapped();
#else
    // ABGR -> GBAR
    QImage i(scaled.size(), scaled.format());
    for (int y = 0; y < i.height(); ++y) {
        QRgb *p = (QRgb*) scaled.scanLine(y);
        QRgb *q = (QRgb*) i.scanLine(y);
        QRgb *end = p + scaled.width();
        while (p < end) {
            *q = qRgba(qGreen(*p), qBlue(*p), qAlpha(*p), qRed(*p));
            p++;
            q++;
        }
    }
#endif

    arg.beginStructure();
    arg << i.width();
    arg << i.height();
    arg << i.bytesPerLine();
    arg << i.hasAlphaChannel();
    int channels = i.isGrayscale() ? 1 : (i.hasAlphaChannel() ? 4 : 3);
    arg << i.depth() / channels;
    arg << channels;
    arg << QByteArray(reinterpret_cast<const char*>(i.bits()), i.byteCount());
    arg.endStructure();
    return arg;
}

/**
 * @brief operator >>
 * @param arg
 * @param img
 * @return
 */
const QDBusArgument& operator>>(const QDBusArgument& arg, QImage &img) {

    arg.beginStructure();

    int width = qdbus_cast<int>(arg);
    int height = qdbus_cast<int>(arg);
    int bytesPerLine = qdbus_cast<int>(arg);
    bool hasAlphaChannel = qdbus_cast<bool>(arg);
    int mult = qdbus_cast<int>(arg);
    int channels = qdbus_cast<int>(arg);
    QByteArray data = qdbus_cast<QByteArray>(arg);

    arg.endStructure();

    img = QImage((uchar*) data.data(), width, height, bytesPerLine, QImage::Format_ARGB32);
    img = img.rgbSwapped();

    return arg;
}

#endif // NOTIFICATIONUTILS_H
