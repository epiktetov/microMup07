#!qmake -- Qt4/Qt5 project file for µMup07 app (for MacOS Х / Linux / Windows)
TEMPLATE = app
CONFIG += release
# -----------------------------------------------------------------------------
#                                platform-dependent location of the Lua library
macx {
  INCLUDEPATH += /opt/local/include # for Lua
  LIBS += /opt/local/lib/liblua.a -framework Cocoa
} else:unix {
  INCLUDEPATH += /usr/include/lua5.2
  LIBS += -llua5.2
} else:win32 {
  INCLUDEPATH += ../lua/include
  LIBS += -L../lua/lib -llua
}
# -----------------------------------------------------------------------------
!macx:TARGET = mim
macx {
  TARGET = µMup07
  ICON = microMir.icns
  QMAKE_INFO_PLIST = mim.Info.plist
  QMAKE_PKGINFO_TYPEINFO = "~epi"
}
QMAKE_CFLAGS   += -std=c99
!win32:DEFINES += UNIX
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HEADERS += micro.keys mic.h mim.h   ccd.h   qfs.h   twm.h   clip.h   synt.h
SOURCES +=                  mim.cpp ccd.cpp qfs.cpp twm.cpp clip.cpp synt.cpp
HEADERS += macs.h
macx:OBJECTIVE_SOURCES += macs.mm
HEADERS += unix.h   luas.h   vip.h   te.h le.h tx.h dq.h ud.h
SOURCES += unix.cpp luas.cpp vip.cpp te.c le.c tx.c dq.c ud.c rt.c
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = obj
RESOURCES = qtmim.qrc
win32:RC_FILE = qtmim.rc
# - - - - - - - - - - - - - - - -
greaterThan(QT_MAJOR_VERSION,4) {
  QT += widgets
}
version.target = version.h
version.depends = .git $$HEADERS $$SOURCES
version.commands = @echo \"const char microVERSION[]=\\\"\"\"`git describe --tags --dirty`\"\"\\\";\" >version.h;git status
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
macx:equals(QT_MAJOR_VERSION,4) {
  MimEXE = µMup07.app/Contents/MacOS/µMup07
  MimFWK = µMup07.app/Contents/Frameworks
  MimRES = µMup07.app/Contents/Resources
  qtconf.target = $$MimRES/qt.conf
  qtconf.depends = qt4.conf
  qtconf.commands = cp $$qtconf.depends $$qtconf.target
  QMAKE_EXTRA_TARGETS += qtconf
  PRE_TARGETDEPS += $$qtconf.target

  QtCORE4 = QtCore.framework/Versions/4
  QtGUI4  = QtGui.framework/Versions/4
  QtCOREname = @executable_path/../Frameworks/$$QtCORE4/QtCore
  QtGUIname  = @executable_path/../Frameworks/$$QtGUI4/QtGui
  qt_nib.target = $$MimRES/qt_menu.nib
  qt_gui.target = $$MimFWK/$$QtGUI4/QtGui
  qtcore.target = $$MimFWK/$$QtCORE4/QtCore
  qt_nib.depends = $$QMAKE_LIBDIR_QT/$$QtGUI4/Resources/qt_menu.nib
  qt_gui.depends = $$QMAKE_LIBDIR_QT/$$QtGUI4/QtGui
  qtcore.depends = $$QMAKE_LIBDIR_QT/$$QtCORE4/QtCore
  qt_nib.commands = cp -R $$qt_nib.depends $$MimRES/
  qt_gui.commands = mkdir -p $$MimFWK/$$QtGUI4;\
                    cp $$qt_gui.depends $$qt_gui.target;\
    install_name_tool -id $$QtGUIname   $$qt_gui.target;\
    install_name_tool -change $$QtCORE4/QtCore $$QtCOREname $$qt_gui.target

  qtcore.commands = mkdir -p $$MimFWK/$$QtCORE4;\
                    cp $$qtcore.depends $$qtcore.target;\
    install_name_tool -id $$QtCOREname  $$qtcore.target

  qtbundle.target = bundle
  qtbundle.depends = all $$qtcore.target $$qt_gui.target $$qt_nib.target
  qtbundle.commands = \
     install_name_tool -change $$QtCORE4/QtCore $$QtCOREname $$MimEXE;\
     install_name_tool -change $$QtGUI4/QtGui   $$QtGUIname  $$MimEXE

  QMAKE_EXTRA_TARGETS += qtcore qt_gui qt_nib qtbundle
}
# -----------------------------------------------------------------------------
