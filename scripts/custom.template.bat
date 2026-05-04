@echo off
:: --- Пути к библиотекам ---
set "QTDIR=E:\Qt\6.10.2\msvc2022_64"
:: Если понадобится, сюда же можно добавить другие пути:
:: set "OpenCASCADE_DIR=C:\OpenCASCADE-7.8.0\opencascade-7.8.0"

:: --- Настройка PATH ---
:: Сразу прокидываем нужные папки в PATH, чтобы и запуск, и сборка всё видели
set "PATH=%QTDIR%\bin;%PATH%"
:: set "PATH=%OpenCASCADE_DIR%\win64\vc14\bin;%PATH%"

:: --- Настройки генерации по умолчанию ---
set "DEFAULT_VS_YEAR=2026"