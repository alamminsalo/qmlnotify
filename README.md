#qmlnotify

Dbus notification server which implements org.freedesktop.Notifications for desktop notifications

##Usage

* Start the server 
* Test via notify-send or similar
* Write your own qml component and start the server with param `--qml $YOUR_QML_NOTIFICATION_COMPONENT`
* Enjoy!

##Screenshots
![Shark png](https://raw.githubusercontent.com/alamminsalo/qmlnotify/master/screenshots/screenshot.png)
![Terminal gif](https://raw.githubusercontent.com/alamminsalo/qmlnotify/master/screenshots/terminal.gif)

##Building instructions

####Install needed packages (arch examples)

```
sudo pacman -S qt5-base
```

####Clone from github and compile
```
git clone https://github.com/alamminsalo/qmlnotify
cd qmlnotify
mkdir build && cd build
qmake ../
make
```

####Test it
```
./qmlnotify
```


