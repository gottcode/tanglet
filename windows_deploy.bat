@ECHO ON>windows\dirs.nsh
@ECHO ON>windows\files.nsh
@ECHO OFF

SET APP=Tanglet
SET VERSION=1.5.2

ECHO Copying executable
MKDIR %APP%
COPY release\%APP%.exe %APP% >nul
strip %APP%\%APP%.exe

ECHO Copying data
MKDIR %APP%\data
XCOPY /Q /S /Y data %APP%\data >nul

ECHO Copying translations
SET TRANSLATIONS=%APP%\translations
MKDIR %TRANSLATIONS%
COPY translations\*.qm %TRANSLATIONS% >nul

ECHO Copying Qt
%QTDIR%\bin\windeployqt.exe --verbose 0 --release --no-angle --no-opengl-sw --no-svg %APP%\%APP%.exe
RMDIR /S /Q %APP%\imageformats

ECHO Creating ReadMe
TYPE README >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
ECHO CREDITS >> %APP%\ReadMe.txt
ECHO ======= >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
TYPE CREDITS >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
ECHO NEWS >> %APP%\ReadMe.txt
ECHO ==== >> %APP%\ReadMe.txt
ECHO. >> %APP%\ReadMe.txt
TYPE NEWS >> %APP%\ReadMe.txt

ECHO Creating installer
CD %APP%
SETLOCAL EnableDelayedExpansion
SET "parentfolder=%__CD__%"
FOR /R . %%F IN (*) DO (
  SET "var=%%F"
  ECHO Delete "$INSTDIR\!var:%parentfolder%=!" >> ..\windows\files.nsh
)
FOR /R /D %%F IN (*) DO (
  TYPE ..\windows\dirs.nsh > temp.txt
  SET "var=%%F"
  ECHO RMDir "$INSTDIR\!var:%parentfolder%=!" > ..\windows\dirs.nsh
  TYPE temp.txt >> ..\windows\dirs.nsh
)
DEL temp.txt
ENDLOCAL
CD ..
makensis.exe /V0 windows\installer.nsi

ECHO Cleaning up
RMDIR /S /Q %APP%
DEL windows\dirs.nsh
DEL windows\files.nsh
