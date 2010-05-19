@ECHO OFF
ECHO Copying executable
MKDIR Tanglet
COPY release\Tanglet.exe Tanglet
strip Tanglet\Tanglet.exe
ECHO Copying dice and word lists
MKDIR Tanglet\data
MKDIR Tanglet\data\en
MKDIR Tanglet\data\fr
XCOPY /S data Tanglet\data
ECHO Copying libraries
COPY %QTDIR%\bin\libgcc_s_dw2-1.dll Tanglet
COPY %QTDIR%\bin\mingwm10.dll Tanglet
COPY %QTDIR%\bin\QtCore4.dll Tanglet
COPY %QTDIR%\bin\QtGui4.dll Tanglet
