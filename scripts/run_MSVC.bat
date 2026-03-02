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

if not exist build\OOP.sln (
    echo [ОШИБКА] Файл решения не найден! Сначала запустите scripts\generate.bat.
    pause
    exit /b 1
)

set "PATH=%QTDIR%\bin;%PATH%"

echo ==== Запуск Visual Studio ====
start devenv build\OOP.sln