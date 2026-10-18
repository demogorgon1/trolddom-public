@echo off
setlocal

rem Exports the layers of the .xcf file in this directory to the PNG files referenced by the map.
rem Expects tpublic-util to have been built as part of kaos (build-dev, RelWithDebInfo).

set TPUBLIC_UTIL=%~dp0..\..\..\..\trolddom-public-build\util\RelWithDebInfo\tpublic-util.exe

if not exist "%TPUBLIC_UTIL%" (
	echo Error: tpublic-util.exe not found. Build tpublic-util ^(RelWithDebInfo^) in kaos build-dev first.
	echo Expected: %TPUBLIC_UTIL%
	pause
	exit /b 1
)

"%TPUBLIC_UTIL%" -export_map_xcf "%~dp0."

if errorlevel 1 (
	pause
	exit /b 1
)
