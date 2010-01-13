@ECHO OFF
ECHO Copying executable
MKDIR Tanglet
COPY release\Tanglet.exe Tanglet
STRIP Tanglet\Tanglet.exe
ECHO Copying dice and word lists
MKDIR Tanglet\data
MKDIR Tanglet\data\en
MKDIR Tanglet\data\fr
COPY data\en\dice Tanglet\data\en
COPY data\en\words Tanglet\data\en
COPY data\fr\dice Tanglet\data\fr
COPY data\fr\words Tanglet\data\fr
ECHO Copying libraries
COPY %QTDIR%\bin\libgcc_s_dw2-1.dll Tanglet
COPY %QTDIR%\bin\mingwm10.dll Tanglet
COPY %QTDIR%\bin\QtCore4.dll Tanglet
COPY %QTDIR%\bin\QtGui4.dll Tanglet
