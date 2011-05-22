#!/bin/sh
# shell script to make µMup07.Qt.app (with Qt framework bundled), Qt 4.5+
rm -rf           µMup07.Qt.app
cp -R µMup07.app µMup07.Qt.app
QtCORE4=QtCore.framework/Versions/4
QtGUI4=QtGui.framework/Versions/4
MimConts=µMup07.Qt.app/Contents
MimQtCORE=$MimConts/Frameworks/$QtCORE4
MimQtGUI=$MimConts/Frameworks/$QtGUI4
mkdir -p $MimQtCORE
mkdir -p $MimQtGUI
LibQtCORE=/Library/Frameworks/$QtCORE4
LibQtGUI=/Library/Frameworks/$QtGUI4
cp    $LibQtCORE/QtCore               $MimQtCORE
cp    $LibQtGUI/QtGui                 $MimQtGUI
cp -R $LibQtGUI/Resources/qt_menu.nib $MimConts/Resources
QtCOREname=@executable_path/../Frameworks/$QtCORE4/QtCore
QtGUIname=@executable_path/../Frameworks/$QtGUI4/QtGui
install_name_tool -id $QtCOREname $MimQtCORE/QtCore
install_name_tool -id $QtGUIname  $MimQtGUI/QtGui
MimEXE=$MimConts/MacOS/µMup07
install_name_tool -change $QtCORE4/QtCore $QtCOREname $MimQtGUI/QtGui
install_name_tool -change $QtCORE4/QtCore $QtCOREname $MimEXE
install_name_tool -change $QtGUI4/QtGui   $QtGUIname  $MimEXE
