@echo off
setlocal

ejs --cmd "App.exeDir.portable" >jem.dir
set /p DIR= <jem.dir
del jem.dir

call "ejs" %DIR%/jem %*
