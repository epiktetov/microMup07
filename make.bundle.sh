#!/bin/sh
# shell script to make µMup07.Qt.app (with Qt framework bundled), old Qt 4.3.3
rm -rf           µMup07.Qt.app
cp -R µMup07.app µMup07.Qt.app
QtCORE4=QtCore.framework/Versions/4
QtGUI4=QtGui.framework/Versions/4
MimQtCORE=µMup07.Qt.app/Contents/Frameworks/$QtCORE4
MimQtGUI=µMup07.Qt.app/Contents/Frameworks/$QtGUI4
mkdir -p $MimQtCORE
mkdir -p $MimQtGUI
LibQtCORE=/usr/local/Qt/lib/$QtCORE4/QtCore
LibQtGUI=/usr/local/Qt/lib/sQtGUI4/QtGui
cp $LibQtCORE $MimQtCORE
cp $LibQtGUI  $MimQtGUI
QtCOREname=@executable_path/../Frameworks/$QtCORE4/QtCore
QtGUIname=@executable_path/../Frameworks/$QtGUI4/QtGui
install_name_tool -id $QtCOREname $MimQtCORE/QtCore
install_name_tool -id $QtGUIname  $MimQtGUI/QtGui
MimEXE=µMup07.Qt.app/Contents/MacOS/µMup07
install_name_tool -change $LibQtCORE $QtCOREname $MimQtGUI/QtGui
install_name_tool -change $LibQtCORE $QtCOREname $MimEXE
install_name_tool -change $LibQtGUI  $QtGUIname  $MimEXE
