@echo off
chcp 65001 >nul

cd /d "%~dp0.."

if exist scripts\custom.bat (
    call scripts\custom.bat
) else (
    echo [ОШИБКА] Файл scripts\custom.bat не найден!
    pause
    exit /b 1
)

if not exist build\CMakeCache.txt (
    echo [ОШИБКА] Кэш CMake не найден! Сначала запустите scripts\generate.bat.
    pause
    exit /b 1
)

:: Читаем использованный генератор напрямую из кэша CMake
set "VS_YEAR=2026"
>nul findstr /C:"Visual Studio 18 2026" build\CMakeCache.txt && set "VS_YEAR=2026"
>nul findstr /C:"Visual Studio 17 2022" build\CMakeCache.txt && set "VS_YEAR=2022"
>nul findstr /C:"Visual Studio 16 2019" build\CMakeCache.txt && set "VS_YEAR=2019"

:: Настраиваем расширение и диапазон версий для vswhere
if "%VS_YEAR%"=="2022" (
    set "SLN_EXT=sln"
    set "VS_VER_RANGE=[17.0,18.0)"
) else if "%VS_YEAR%"=="2019" (
    set "SLN_EXT=sln"
    set "VS_VER_RANGE=[16.0,17.0)"
) else (
    set "VS_YEAR=2026"
    set "SLN_EXT=slnx"
    set "VS_VER_RANGE=[18.0,19.0)"
)

if not exist build\OOP.%SLN_EXT% (
    echo [ОШИБКА] Файл решения build\OOP.%SLN_EXT% не найден!
    pause
    exit /b 1
)

echo ==== Поиск и запуск Visual Studio %VS_YEAR% ====
set "VSWHERE_DIR=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
set "DEVENV_PATH="

:: Переходим в папку с vswhere для обхода бага cmd с пробелами в путях
if exist "%VSWHERE_DIR%\vswhere.exe" (
    pushd "%VSWHERE_DIR%"
    for /f "usebackq tokens=*" %%i in (`vswhere.exe -version "%VS_VER_RANGE%" -property productPath`) do set "DEVENV_PATH=%%i"
    popd
)

if defined DEVENV_PATH (
    start "" "%DEVENV_PATH%" build\OOP.%SLN_EXT%
) else (
    echo [ОШИБКА] Установка Visual Studio %VS_YEAR% не найдена в системе!
    echo [ИНФО] Попытка аварийного запуска через системный devenv...
    start devenv build\OOP.%SLN_EXT%
)