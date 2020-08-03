Qt4 or Qt5 (http://download.qt.io/archive/qt/) and Lua (http://www.lua.org/)
are requred and must be installed before the build (please check that file
qtmim.pro points to correct location of Lua libraries, fix INCLUDEPATH and
LIBS if it does not). After that, building is simple:

	qmake qtmim.pro && make

In order to make the macOS application bundle not dependent on external
Qt frameworks, run the following command:

	make bundle

For help with the editing refer to `micro.keys` file in this repository,
or on-line at http://www.epiktetov.com/micro-keys (may be not up-to-date)
