cp µMup07.app/Contents/MacOS/µMup07 µMup07.Qt.app/Contents/MacOS/µMup07
rm                                  µMup07.Qt.app/Contents/Resources/*
cp µMup07.app/Contents/Resources/*  µMup07.Qt.app/Contents/Resources
#
#install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui   µMup07.Qt.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
#install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore µMup07.Qt.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
install_name_tool -change /usr/local/Qt/lib/QtGui.framework/Versions/4/QtGui   @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui   µMup07.Qt.app/Contents/MacOS/µMup07
install_name_tool -change /usr/local/Qt/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore µMup07.Qt.app/Contents/MacOS/µMup07
#install_name_tool -change /usr/local/Qt/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore µMup07.Qt.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
