@echo off
chcp 65001 >nul

cd /d "%~dp0.."

echo ==== Подготовка окружения ====
if exist scripts\custom.bat (
    call scripts\custom.bat
) else (
    echo [ОШИБКА] Файл scripts\custom.bat не найден!
    pause
    exit /b 1
)

if not defined DEFAULT_VS_YEAR set "DEFAULT_VS_YEAR=2026"
set "VS_YEAR=%DEFAULT_VS_YEAR%"

if not "%~1"=="" set "VS_YEAR=%~1"

if "%VS_YEAR%"=="2026" (
    set "CMAKE_GEN=Visual Studio 18 2026"
) else if "%VS_YEAR%"=="2022" (
    set "CMAKE_GEN=Visual Studio 17 2022"
) else if "%VS_YEAR%"=="2019" (
    set "CMAKE_GEN=Visual Studio 16 2019"
) else (
    echo [ПРЕДУПРЕЖДЕНИЕ] Неизвестная версия "%VS_YEAR%", откатываемся на 2026.
    set "VS_YEAR=2026"
    set "CMAKE_GEN=Visual Studio 18 2026"
)

echo.
echo ==== Генерация CMake для Visual Studio %VS_YEAR% ====
cmake -G "%CMAKE_GEN%" -A x64 -S . -B build -DCMAKE_PREFIX_PATH="%QTDIR%"

if %ERRORLEVEL% NEQ 0 (
    echo [ОШИБКА] Ошибка генерации CMake!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [УСПЕХ] Проект сгенерирован в корневую папку build!
pause