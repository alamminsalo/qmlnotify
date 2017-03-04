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

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusAbstractAdaptor>

class NotificationManager: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    NotificationManager(QQmlApplicationEngine *engine, const QString &path, QObject *parent = 0);
    virtual ~NotificationManager();

private slots:
    void triggerNext();

public slots:
    Q_NOREPLY void Notify(const QDBusMessage &msg);

private:
    QQmlApplicationEngine *engine;
    QList<QVariantMap> queue;

    //Currently visible notification
    QObject *currentObject;
    QQmlComponent *component;

    QDBusInterface *interf;
};

#endif // NOTIFICATIONMANAGER_H
