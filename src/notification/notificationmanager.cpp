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

NotificationManager::NotificationManager(QQmlApplicationEngine *engine, const QString &path, QObject *parent) : QDBusAbstractAdaptor(parent)
{
    this->engine = engine;
    component = new QQmlComponent(this->engine, QUrl(path));
    component->setParent(this);
    currentObject = 0;
    queue.clear();

    //Setup dbus listener
    QDBusConnection::sessionBus().connect("", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify", this, SLOT(notify(const QDBusMessage &)));
    QDBusConnection::sessionBus().registerObject("/org/freedesktop/Notifications", parent);

    QString matchString = "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'";
    interf = new QDBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);
    interf->call("AddMatch", matchString);

    qDebug() << "started listener.";
}


NotificationManager::~NotificationManager()
{
}

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
            properties["hints"]       = msg.arguments().at(i).toString();
            break;
        case 7:
            properties["timeout"]     = msg.arguments().at(i).toInt();
        }
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
