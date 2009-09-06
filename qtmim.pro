TEMPLATE = app
macx {
  CONFIG += x86
  TARGET = µMup07
  ICON = microMir.icns
  QMAKE_INFO_PLIST = mim.Info.plist
  QMAKE_PKGINFO_TYPEINFO = "~epi"
}
!macx:TARGET = mim
win32:RC_FILE = qtmim.rc
!win32:DEFINES += UNIX

# Input
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += mic.h mim.h   ccd.h micro.keys qfs.h   twm.h   vip.h   clip.h
SOURCES +=  rt.c mim.cpp ccd.cpp          qfs.cpp twm.cpp vip.cpp clip.cpp
!win32 {
  HEADERS += unix.h
  SOURCES += unix.cpp
}
HEADERS += te.h le.h tx.h dq.h ud.h
SOURCES += te.c le.c tx.c dq.c ud.c
OBJECTS_DIR = obj

# Customization
macx {
  MimRESOURCES = µMup07.app/Contents/Resources
  rsrc.target = $$MimRESOURCES/micros.icns $$MimRESOURCES/micro.keys
  rsrc.depends =               micros.icns                micro.keys
  rsrc.commands = @mkdir -p $$MimRESOURCES; cp $$rsrc.depends $$MimRESOURCES
  POST_TARGETDEPS += $$rsrc.target
  QMAKE_EXTRA_TARGETS += rsrc

  MimRESOURCES = µMup07.app/Contents/Resources
  icns.target = $$MimRESOURCES/micros.icns
  keys.target = $$MimRESOURCES/micro.keys
  icns.depends = micros.icns
  keys.depends = micro.keys
  icns.commands = @mkdir -p $$MimRESOURCES; cp $$icns.depends $$icns.target
  keys.commands = @mkdir -p $$MimRESOURCES; cp $$keys.depends $$keys.target
  POST_TARGETDEPS += $$icns.target $$keys.target
  QMAKE_EXTRA_TARGETS += icns keys
}
MimFILES  = micros.dir qtmim.pro qtmim.desktop qtmim.rc mim.Info.plist
MimFILES += micons.psd qtmim.ico qtmim.png
zip.target  = zip
zip.depends = Qtmim.zip

zipfile.target = Qtmim.zip
zipfile.depends = $$MimFILES $$HEADERS $$SOURCES
zipfile.commands = rm -f $$zipfile.target; zip $$zipfile.target $$zipfile.depends

QMAKE_EXTRA_TARGETS += zip zipfile
