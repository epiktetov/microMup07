Qt (http://qt-project.org/) and Lua (http://www.lua.org/) are requred for the
build (only tested with Qt 4.8, but hopefully later version will work as well).
Once they are installed in their default locations, use this command:

> qmake qtmim.pro && make

For building full application bundle on Mac OS X (with Qt framework included)
additional step is required:

> make bundle

For help with the editing (and you will need it, trust me) see `micro.keys`
file in this repository, or http://www.epiktetov.com/micro-keys on my site
