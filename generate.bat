@echo off
echo Updating submodules...
call git submodule update --init --recursive
call tools\premake5 %* vs2022 --copy-to="D:\SteamLibrary\steamapps\common\Call of Duty Modern Warfare 2"
