@echo off
setlocal

for /f "tokens=*" %%a in ('ejs --cmd "App.exeDir.portable"') do set BIN=%%a
call "%BIN%/ejs" %BIN%/bit %*
