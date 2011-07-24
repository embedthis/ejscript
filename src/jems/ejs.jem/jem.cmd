@echo off
setlocal

ejs --cmd "App.exeDir.portable" >mvc.dir
set /p DIR= <mvc.dir
del mvc.dir

call "ejs" %DIR%/jem %*
