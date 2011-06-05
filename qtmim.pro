#!qmake -- Qt project file for µMup07
TEMPLATE = app
CONFIG += release
unix:QtPLATF = unix# must be the first (as 'unix' condition's true on macx too)
macx {
  TARGET = µMup07
  ICON = microMir.icns
  QtPLATF = macx
  QMAKE_INFO_PLIST = mim.Info.plist
  QMAKE_PKGINFO_TYPEINFO = "~epi"
}
!macx:TARGET = mim
win32 {
  RC_FILE = qtmim.rc
  QtPLATF = win32
}
!win32:DEFINES += UNIX
QMAKE_CXXFLAGS += -DQtPLATF=\'\"$$QtPLATF\"\'

HEADERS += micro.keys mic.h mim.h   ccd.h   qfs.h   twm.h   clip.h   synt.h
SOURCES +=                  mim.cpp ccd.cpp qfs.cpp twm.cpp clip.cpp synt.cpp
HEADERS += macs.h
macx:OBJECTIVE_SOURCES += macs.mm
macx:LIBS += -framework Cocoa
!win32 {
  HEADERS += unix.h
  SOURCES += unix.cpp
}
HEADERS += vip.h   te.h le.h tx.h dq.h ud.h
SOURCES += vip.cpp te.c le.c tx.c dq.c ud.c rt.c
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = obj
RESOURCES = qtmim.qrc

version.target = version.h
version.depends = .git $$HEADERS $$SOURCES
version.commands = @echo \"const char microVERSION[]=\\\"\"\"`git describe --tags --dirty`\"\"\\\";\" >version.h;git status
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h
macx {
  MimEXECUTABLE = µMup07.app/Contents/MacOS/µMup07
  MimRESOURCES = µMup07.app/Contents/Resources
  icns.target = $$MimRESOURCES/micros.icns
  icns.depends = micros.icns
  icns.commands = @mkdir -p $$MimRESOURCES; cp $$icns.depends $$icns.target
  symlink.target = mim
  symlink.commands = ln -sf $$MimEXECUTABLE $$symlink.target
  QMAKE_EXTRA_TARGETS += icns symlink
  POST_TARGETDEPS += $$icns.target $$symlink.target
#
# For some reason, QtCreator cannot handle qt_menu.nib by itself, when building
# with debug library (which are not available in framework format), assisting..
#
  qt_menu.target = $$MimRESOURCES/qt_menu.nib
  qt_menu.depends = ../installs/Qt-sources-git/src/gui/mac/qt_menu.nib
  qt_menu.commands = cp -R $$qt_menu.depends $$MimRESOURCES
  qt_menu_ext.target = qt_menu
  qt_menu_ext.depends = $$qt_menu.target
  QMAKE_EXTRA_TARGETS += qt_menu qt_menu_ext
}
MimFILES  = LICENSE    micros.dir microMir.icns micros.icns mim.Info.plist
MimFILES += micro.keys micons.psd microMir.png
MimFILES +=              qtmim.pro qtmim.desktop qtmim.rc abc
MimFILES += Qt.bundle.sh qtmim.ico qtmim.png keywords.txt test.txt
zip.target  = zip
zip.depends = Qtmim.zip
zipfile.target = Qtmim.zip
zipfile.depends = $$MimFILES $$HEADERS $$SOURCES $$RESOURCES
zipfile.commands = rm -f $$zipfile.target; zip $$zipfile.target $$zipfile.depends
QMAKE_EXTRA_TARGETS += zip zipfile
