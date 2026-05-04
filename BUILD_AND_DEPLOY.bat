@echo off
setlocal

echo === Building LevelUtilityTool Editor Plugin ===

set "PROJECT_ROOT=%~dp0"
set "UNIGINE_SDK_DIR=C:\Users\snare\AppData\Local\unigine\browser\sdks\sim_windows_2.18.1_bin"
set "UNIGINE_PROJECT_DIR=C:\Users\snare\Documents\UNIGINE Projects\unigine_project_3"
set "QT_DIR=C:\Qt\Qt5.12.3\5.12.3\msvc2017_64"
set "BUILD_DIR=%PROJECT_ROOT%build_sim_2_18_1"
set "DEPLOY_DIR=%UNIGINE_PROJECT_DIR%\bin\plugins\Vamps\LevelUtilityTool"
set "TARGET_NAME=LevelUtilityTool"
set "BINARY_NAME=%TARGET_NAME%_editorplugin_double_x64"

echo Project Root:       %PROJECT_ROOT%
echo Build Dir:          %BUILD_DIR%
echo Deploy Dir:         %DEPLOY_DIR%
echo UNIGINE SDK:        %UNIGINE_SDK_DIR%
echo UNIGINE Project:    %UNIGINE_PROJECT_DIR%
echo Qt Dir:             %QT_DIR%
echo.

if not exist "%UNIGINE_SDK_DIR%\include\Unigine.h" (
	echo ERROR: UNIGINE SDK not found at "%UNIGINE_SDK_DIR%".
	exit /b 1
)

if not exist "%UNIGINE_PROJECT_DIR%\unigine_project_3.project" (
	echo ERROR: UNIGINE project not found at "%UNIGINE_PROJECT_DIR%".
	exit /b 1
)

if not exist "%QT_DIR%\lib\cmake\Qt5\Qt5Config.cmake" (
	echo ERROR: Qt5Config.cmake not found at "%QT_DIR%\lib\cmake\Qt5".
	echo Update QT_DIR in this batch file if your Qt path is different.
	exit /b 1
)

set "VCVARS64="
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS64=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not defined VCVARS64 if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS64=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not defined VCVARS64 if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS64=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not defined VCVARS64 if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS64=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not defined VCVARS64 if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS64=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat"

if not defined VCVARS64 (
	echo ERROR: vcvars64.bat was not found. Install Visual Studio Build Tools or update this batch file.
	exit /b 1
)

echo Setting up Visual Studio environment...
call "%VCVARS64%"
if errorlevel 1 (
	echo ERROR: Failed to initialize Visual Studio environment.
	exit /b 1
)

set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Links;%QT_DIR%\bin;%PATH%"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

echo.
echo Step 1: Configuring CMake...
if exist "CMakeCache.txt" del /f /q "CMakeCache.txt"
if exist "CMakeFiles" rmdir /s /q "CMakeFiles"

cmake "%PROJECT_ROOT%." -G "Ninja" -DCMAKE_BUILD_TYPE=Release ^
	-DUNIGINE_DOUBLE=ON ^
	-DUNIGINE_SDK_DIR="%UNIGINE_SDK_DIR%" ^
	-DQT5_DIR="%QT_DIR%" ^
	-DQT_DIR="%QT_DIR%" ^
	-DDEPLOY_BASE_DIR="%UNIGINE_PROJECT_DIR%" ^
	-DDEPLOY_DIR="%DEPLOY_DIR%" ^
	-DCMAKE_PREFIX_PATH="%QT_DIR%" ^
	-DQt5_DIR="%QT_DIR%\lib\cmake\Qt5"

if errorlevel 1 (
	popd
	echo ERROR: CMake configuration failed.
	exit /b 1
)

echo.
echo Step 2: Building and deploying...
ninja -v
if errorlevel 1 (
	popd
	echo ERROR: Build failed.
	exit /b 1
)

popd

echo.
if exist "%DEPLOY_DIR%\%BINARY_NAME%.dll" (
	echo [OK] Plugin DLL: "%DEPLOY_DIR%\%BINARY_NAME%.dll"
) else (
	echo [WARN] Expected DLL was not found at "%DEPLOY_DIR%\%BINARY_NAME%.dll".
)

if exist "%DEPLOY_DIR%\plugin.json" echo [OK] plugin.json deployed.
if exist "%DEPLOY_DIR%\LevelUtilityTool.json" echo [OK] Qt plugin metadata deployed.
if exist "%DEPLOY_DIR%\LevelUtilityTool.plugin" echo [OK] Legacy plugin descriptor deployed.

echo.
echo === Build Complete ===
echo Start the UNIGINE editor for unigine_project_3 and open the Level Utility Tool panel.
