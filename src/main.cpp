#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "notification/notificationmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QGuiApplication::setQuitOnLastWindowClosed(false);

    //Use reference implementation as default notification
    QString component_path = "qrc:/Notification.qml";

    for (int i = 0; i < argc; i++) {
        if (QString(argv[i]) == "--qml") {
            if (++i < argc) {
                component_path = QString(argv[i]);
            }
        }
    }

    QQmlApplicationEngine engine;

    QObject obj;
    NotificationManager *mgr = new NotificationManager(&engine, component_path, &obj);
    Q_UNUSED(mgr);

    return app.exec();
}
