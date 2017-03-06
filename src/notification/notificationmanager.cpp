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
#include <QIcon>

#include "notificationutils.h"

NotificationManager::NotificationManager(QQmlApplicationEngine *engine, const QString &path, QObject *parent) : QDBusAbstractAdaptor(parent)
{
    this->engine = engine;
    component = new QQmlComponent(this->engine, QUrl(path));
    component->setParent(this);
    currentObject = 0;
    queue.clear();

    //Setup dbus listener
    if (!QDBusConnection::sessionBus().registerService("org.freedesktop.Notifications")) {
        qDebug() << "Warning: Couldn't register service org.freedesktop.Notifications";
    }
    if (!QDBusConnection::sessionBus().registerObject("/org/freedesktop/Notifications", parent)) {
        qDebug() << "Error: Couldn't register handler for org.freedesktop.Notifications";
    }

    QString matchString = "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'";
    QDBusInterface *interf = new QDBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);
    interf->call("AddMatch", matchString);


    qDebug() << "started listener.";

    qDBusRegisterMetaType<QImage>();

    setAutoRelaySignals(true);
}

NotificationManager::~NotificationManager()
{
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

    queue.append(parseMessage(msg));

    if (!currentObject){
        triggerNext();
    }
}

QString NotificationManager::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    //TODO: check for use case
    return "";
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

QVariantMap NotificationManager::parseMessage(const QDBusMessage &msg)
{
    QVariantMap properties;

    for (int i=0; i < msg.arguments().size(); i++) {

        switch(i) {
        case 0:
            properties["app_name"]    = msg.arguments().at(i);
            break;
        case 1:
            properties["app_id"]      = msg.arguments().at(i);
            break;
        case 2:
            properties["icon"]        = msg.arguments().at(i);
            break;
        case 3:
            properties["summary"]     = msg.arguments().at(i);
            break;
        case 4:
            properties["body"]        = msg.arguments().at(i);
            break;
        case 5:
            properties["actions"]     = msg.arguments().at(i);
            break;
        case 6:
            properties["hints"]       = msg.arguments().at(i);
            break;
        case 7:
            properties["timeout"]     = msg.arguments().at(i);
        }
    }

    //Parse hints
    if (!properties["hints"].isNull()) {
        QVariantMap hints = qdbus_cast<QVariantMap>(*(static_cast<QDBusArgument*>((void *)properties["hints"].data())));

        if (!hints.isEmpty()) {
            QImage img = qdbus_cast<QImage>(hints["image_data"]);

            if (!img.isNull())
                properties["image_data"] =  utils::imageToBase64(img);

            //TODO: parse rest
        }
    }

    //Check that theme name is set, 'hicolor' can be sign of theme not set or qt having problems it
    QString theme = QIcon::themeName();
    if (theme == "hicolor") {
        qDebug() << "QIcon::themeName() returned 'hicolor', is this right?";
    }

    //If icon doesn't point to a file, attempt to fetch it from theme
    if (!QFile(properties["icon"].toString()).exists()) {
        if (QIcon::hasThemeIcon(properties["icon"].toString().toLower())) {
            QIcon icon = QIcon::fromTheme(properties["icon"].toString().toLower());
            if (!icon.isNull())
                properties["icon"] = utils::imageToBase64(icon.pixmap(128,128).toImage());
        }
    }

    //Attempt to fetch application icon from theme to app_icon variable
    if (QIcon::hasThemeIcon(properties["app_name"].toString().toLower())) {
        QIcon icon = QIcon::fromTheme(properties["app_name"].toString().toLower());
        if (!icon.isNull())
            properties["app_icon"] = utils::imageToBase64(icon.pixmap(128,128).toImage());
    }

    return properties;
}
