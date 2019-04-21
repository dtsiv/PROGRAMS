call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
SET _ROOT=C:\PROGRAMS\qt-everywhere-opensource-src-5.6.3
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;C:\Qt5\jom;C:\Python27;C:\Perl64\bin;%PATH%
REM Uncomment the below line when using a git checkout of the source repository
REM SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET QMAKESPEC=win32-msvc2017
SET _ROOT=
chcp 866
call C:\PROGRAMS\qt-everywhere-opensource-src-5.6.3\configure.bat -verbose -I C:\PROGRAMS\PostgreSQL_9.4_32bit -opengl desktop -opensource -confirm-license -mp -debug-and-release -prefix C:\Qt5\5.6.3 -target xp -xplatform win32-msvc2017 -plugin-sql-psql -plugin-sql-sqlite2 -plugin-sql-sqlite -opengl desktop
PAUSE