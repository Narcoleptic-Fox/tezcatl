@echo off
rem Runs a command inside a Visual Studio x64 developer environment, so the
rem windows-msvc preset works from any shell: scripts\msvc.cmd cmake --preset windows-msvc
setlocal
set "VSINSTALLER=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
rem vcvars itself calls vswhere by bare name, so the installer must be on PATH.
set "PATH=%VSINSTALLER%;%PATH%"
for /f "usebackq delims=" %%i in (`vswhere.exe -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
    echo msvc.cmd: no Visual Studio with the C++ x64 tools was found 1>&2
    exit /b 2
)
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 2
%*
