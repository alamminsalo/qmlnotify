#qmlnotify

![gif1](https://raw.githubusercontent.com/alamminsalo/qmlnotify/master/screenshots/record1.gif)
![gif2](https://raw.githubusercontent.com/alamminsalo/qmlnotify/master/screenshots/record2.gif)

##What is it?
Desktop notification server which implements org.freedesktop.Notifications, using modern Qt/Qml technologies

##Usage

* Start the server 
* Test via notify-send or similar
* Write your own qml component and start the server with param `--qml $YOUR_QML_NOTIFICATION_COMPONENT`
* Enjoy!

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

