TEMPLATE = app
unix:QtPLATF = unix# must be the first (as 'unix' condition's true on macx too)
macx {
  CONFIG += x86
  TARGET = µMup07
  ICON = microMir.icns
  QtPLATF = macx
  QMAKE_INFO_PLIST = mim.Info.plist
  QMAKE_PKGINFO_TYPEINFO = "~epi"
}
!macx:TARGET = mim
win32 {
  CONFIG += release
  RC_FILE = qtmim.rc
  QtPLATF = win32
}
!win32:DEFINES += UNIX
QMAKE_CXXFLAGS += -DQtPLATF=\'\"$$QtPLATF\"\'

HEADERS += micro.keys mic.h mim.h   ccd.h   qfs.h   twm.h   clip.h   synt.h
SOURCES +=                  mim.cpp ccd.cpp qfs.cpp twm.cpp clip.cpp synt.cpp
!win32 {
  HEADERS += unix.h
  SOURCES += unix.cpp
}
HEADERS += vip.h   te.h le.h tx.h dq.h ud.h
SOURCES += vip.cpp te.c le.c tx.c dq.c ud.c rt.c
DEPENDPATH += .
INCLUDEPATH += .
OBJECTS_DIR = obj

version.target = version.h
version.depends = .git $$HEADERS $$SOURCES
version.commands = @echo \"const char microVERSION[]=\\\"\"\"`git describe --tags --dirty`\"\"\\\";\" >version.h;git status
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h
macx {
  MimEXECUTABLE = µMup07.app/Contents/MacOS/µMup07
  MimRESOURCES = µMup07.app/Contents/Resources
  icns.target = $$MimRESOURCES/micros.icns
  keys.target = $$MimRESOURCES/micro.keys
  icns.depends = micros.icns
  keys.depends = micro.keys
  icns.commands = @mkdir -p $$MimRESOURCES; cp $$icns.depends $$icns.target
  keys.commands = @mkdir -p $$MimRESOURCES; cp $$keys.depends $$keys.target
  symlink.target = mim
  symlink.commands = ln -sf $$MimEXECUTABLE $$symlink.target
  QMAKE_EXTRA_TARGETS += icns keys symlink
  POST_TARGETDEPS += $$icns.target $$keys.target $$symlink.target
}
MimFILES  = micros.dir qtmim.pro qtmim.desktop qtmim.rc mim.Info.plist
MimFILES += micons.psd qtmim.ico qtmim.png keywords.txt abc LICENSE
zip.target  = zip
zip.depends = Qtmim.zip
zipfile.target = Qtmim.zip
zipfile.depends = $$MimFILES $$HEADERS $$SOURCES
zipfile.commands = rm -f $$zipfile.target; zip $$zipfile.target $$zipfile.depends
QMAKE_EXTRA_TARGETS += zip zipfile
