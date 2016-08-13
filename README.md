Qt4 or Qt5 (http://download.qt.io/archive/qt/) and Lua (http://www.lua.org/)
are requred and must be installed before the build (please check that file
qtmim.pro points to correct location of Lua libraries, fix INCLUDEPATH and
LIBS if it does not). After that, building is simple:

> qmake qtmim.pro && make

In order to make the MacOS X application bundle not dependent on external
Qt frameworks, run the following command:

> make bundle            # for Qt 4.x (no plugins)
> macdeployqt µMup07.app # for Qt 5.x

For help with the editing (and you will need it, trust me) see `micro.keys`
file in this repository, or on-line at http://www.epiktetov.com/micro-keys
