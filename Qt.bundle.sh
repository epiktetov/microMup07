#!/bin/sh
# shell script to make µMup07.Qt.app (with Qt framework bundled), Qt 4.5+
cp -R µMup07.app µMup07.Qt.app
QtCOREpath=Frameworks/QtCore.framework/Versions/4
QtGUIpath=Frameworks/QtGUI.framework/Versions/4
MimQtCORE=µMup07.Qt.app/Contents/$QtCOREpath
MimQtGUI=µMup07.Qt.app/Contents/$QtGUIpath
mkdir -p $MimQtCORE
mkdir -p $MimQtGUI
LibQtCORE=/Library/Frameworks/QtCore.framework/Versions/4/QtCore
LibQtGUI=/Library/Frameworks/QtGui.framework/Versions/4/QtGui
cp $LibQtCORE $MimQtCORE
cp $LibQtGUI  $MimQtGUI
install_name_tool -change $LibQtCORE @executable_path/../$QtCOREpath/QtCore µMup07.Qt.app/Contents/MacOS/µMup07
install_name_tool -change $LibQtGUI  @executable_path/../$QtGUIpath/QtGui   µMup07.Qt.app/Contents/MacOS/µMup07
