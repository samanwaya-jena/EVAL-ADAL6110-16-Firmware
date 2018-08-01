@echo off

REM IF "%1"=="" GOTO SKIP
REM IF "%2"=="" GOTO SKIP

REM pushd %~dp0

set TargetName=BF707_Wagner
set Configuration=Debug
set TargetExt=.ldr
set CCESDir=d:\Analog Devices\CrossCore Embedded Studio 2.8.0
set DriverPath=d:\Analog Devices\ADSP-BF707_EZ-Board-Rel1.0.1\BF707_EZ-Board\Blackfin\Examples\Device_programmer\bf707_w25q32bv_dpia.dxe

ECHO Programming %TargetName% to SPI Flash

"%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd prog -erase affected -format hex -file %Configuration%\%TargetName%%TargetExt%

REM popd

:DONE
