@echo off
chcp 65001 >nul

cd /d "%~dp0.."

echo ==== Подготовка проекта ====
if exist scripts\custom.bat (
    call scripts\custom.bat
) else (
    echo [ОШИБКА] Файл scripts\custom.bat не найден!
    pause
    exit /b 1
)

echo.
echo ==== Генерация CMake ====
cmake -S . -B build -DCMAKE_PREFIX_PATH="%QTDIR%"

echo.
echo [УСПЕХ] Проект сгенерирован в корневую папку build!
pause