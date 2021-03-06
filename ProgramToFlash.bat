@echo off

REM IF "%1"=="" GOTO SKIP
REM IF "%2"=="" GOTO SKIP

REM pushd %~dp0

set TargetName=demo_app
set Configuration=Debug
set TargetExt=.ldr
set CCESDir=C:\Analog Devices\CrossCore Embedded Studio 2.9.1
set DriverPath=C:\GIT\Wagner\flash_programmer\flash_programmer.dxe
set configFile=wagner_custom_board_support.xml

ECHO Programming %TargetName% to SPI Flash

"%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd prog -erase affected -format hex -file %TargetName%\%Configuration%\%TargetName%%TargetExt% -verbose -log 
REM "%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd info -verbose -log 
REM popd

set DEVICELISTFILE = wagner_list.csv
set LISTPATH = .

ECHO Editing device informations from %DEVICELISTFILE

rem python configdev.py "%LISTPATH%/%DEVICELISTFILE%"

:DONE
