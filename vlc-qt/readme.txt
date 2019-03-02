qt 4.8.4
vlc 2.0.0 (https://download.videolan.org/videolan/vlc/2.0.0/win32/vlc-2.0.0-win32.zip)
vlc-qt 0.8.0 (https://github.com/vlc-qt/vlc-qt/releases/download/0.8.0/libvlc-qt_0.8.0_msvc2010_qt4.zip)
msvc 2008

vlc-qt-env.bat
==============
call "c:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
set LIB_DIR=C:\PROGRAMS\vlc-2.0.0\sdk
cmake ..\src -DCMAKE_INSTALL_PREFIX=C:\PROGRAMS\libvlc-qt_0.8.0_handmade_Dima -Wno-dev -G "Visual Studio 9 2008" -A Win32 -DBUILD_TESTS=ON -DWITH_QML=ON