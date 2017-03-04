/*
 * Copyright © 2015-2016 Antti Lamminsalo
 *
 * This file is part of qmlnotify.
 *
 * qmlnotify is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with qmlnotify.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "notificationmanager.h"
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QImage>

NotificationManager::NotificationManager(QQmlApplicationEngine *engine, const QString &path, QObject *parent) : QDBusAbstractAdaptor(parent)
{
    this->engine = engine;
    component = new QQmlComponent(this->engine, QUrl(path));
    component->setParent(this);
    currentObject = 0;
    queue.clear();

    //Setup dbus listener
    QDBusConnection::sessionBus().registerObject("/org/freedesktop/Notifications", parent);

    QString matchString = "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'";
    interf = new QDBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);
    interf->call("AddMatch", matchString);

    qDebug() << "started listener.";

    qDBusRegisterMetaType<QImage>();
}

NotificationManager::~NotificationManager()
{
}

QByteArray base64Image(const QImage &img)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    img.save(&buffer, "PNG"); // writes the image in PNG format inside the buffer

    return "data:image/png;base64," + byteArray.toBase64();
}

#include <QDBusArgument>
/**
 * @brief NotificationManager::Notify
 * @param msg
 *
 * This function is triggered when org.freedesktop.Notifications sends
 * Notify-signal from dbus
 */
void NotificationManager::Notify(const QDBusMessage &msg) {

    QVariantMap properties;

    for (int i=0; i < msg.arguments().size(); i++) {

        qDebug() << msg.arguments().at(i);

        switch(i) {
        case 0:
            properties["app_name"]    = msg.arguments().at(i).toString();
            break;
        case 1:
            properties["app_id"]      = msg.arguments().at(i).toInt();
            break;
        case 2:
            properties["icon"]        = msg.arguments().at(i).toString();
            break;
        case 3:
            properties["summary"]     = msg.arguments().at(i).toString();
            break;
        case 4:
            properties["body"]        = msg.arguments().at(i).toString();
            break;
        case 5:
            properties["actions"]     = msg.arguments().at(i).toString();
            break;
        case 6:
            properties["hints"]       = msg.arguments().at(i);
            break;
        case 7:
            properties["timeout"]     = msg.arguments().at(i).toInt();
        }
    }

    if (!properties["hints"].isNull()) {
        QVariantMap elems = qdbus_cast<QVariantMap>(*(static_cast<QDBusArgument*>((void *)properties["hints"].data())));

        //TODO: parse rest
        QImage img = qdbus_cast<QImage>(elems["image_data"]);

        if (!img.isNull())
            properties["image_data"] =  base64Image(img);
    }

    queue.append(properties);

    if (!currentObject){
        triggerNext();
    }
}

void NotificationManager::triggerNext()
{
    //Pops first in queue, and shows it
    if (currentObject){
        currentObject->deleteLater();
        currentObject = 0;
    }

    if (!queue.isEmpty()){

        QVariantMap properties = queue.takeFirst();

        if (component->isError()) {
            qDebug() << component->errorString();
        }
        if (component->isReady()) {
            currentObject = component->create(engine->rootContext());

            // Set data to notification
            currentObject->setProperty("properties", properties);

            QObject::connect(currentObject, SIGNAL(timeout()), this, SLOT(triggerNext()));

            // Trigger notification
            currentObject->setProperty("visible", QVariant::fromValue(true));
        }
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

    img = QImage((const uchar*) data.data(), width, height, bytesPerLine, QImage::Format_ARGB32);
    img = img.rgbSwapped();

    return arg;
}
