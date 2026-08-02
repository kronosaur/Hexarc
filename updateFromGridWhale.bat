@echo off
setlocal EnableExtensions

rem Update the Hexarc source directories from the GridWhale stream.
rem By default, GridWhale is expected to be a sibling of the outer Hexarc
rem directory:  Documents\GridWhale and Documents\Hexarc\Hexarc.

for %%I in ("%~dp0.") do set "HEXARC_ROOT=%%~fI"
set "GRIDWHALE_ROOT=%HEXARC_ROOT%\..\..\GridWhale"
set "DRY_RUN="

if not "%~1" == "" (
    if /I "%~1" == "/L" (
        set "DRY_RUN=1"
    ) else (
        set "GRIDWHALE_ROOT=%~f1"
    )
)

if /I "%~2" == "/L" (
    set "DRY_RUN=1"
) else if not "%~2" == "" (
    goto usage
)

if not "%~3" == "" goto usage

for %%I in ("%GRIDWHALE_ROOT%\.") do set "GRIDWHALE_ROOT=%%~fI"

if not exist "%HEXARC_ROOT%\Hexarc.sln" (
    echo ERROR: Hexarc.sln was not found under "%HEXARC_ROOT%".
    exit /b 1
)

if not exist "%GRIDWHALE_ROOT%\GridWhale.sln" (
    echo ERROR: GridWhale.sln was not found under "%GRIDWHALE_ROOT%".
    echo.
    goto usage
)

git -c "safe.directory=%GRIDWHALE_ROOT%" -C "%GRIDWHALE_ROOT%" rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    echo ERROR: "%GRIDWHALE_ROOT%" is not a Git working tree.
    exit /b 1
)

echo Updating common source directories from:
echo   %GRIDWHALE_ROOT%
echo to:
echo   %HEXARC_ROOT%
if defined DRY_RUN echo DRY RUN: No files will be changed.
echo.

set "UPDATE_FAILED="
set /A FILES_UPDATED=0
for /D %%D in ("%HEXARC_ROOT%\*") do call :updateDirectory "%%~nxD" || set "UPDATE_FAILED=1"

echo.
if defined UPDATE_FAILED (
    echo Update failed. See the errors above.
    exit /b 1
)

if defined DRY_RUN (
    echo Dry run completed successfully. %FILES_UPDATED% file^(s^) would be updated.
) else (
    echo Update completed successfully. %FILES_UPDATED% file^(s^) updated.
)
exit /b 0

:updateDirectory
set "DIRECTORY_NAME=%~1"

rem These may exist in both trees, but they are not source directories.
if /I "%DIRECTORY_NAME%" == ".git" exit /b 0
if /I "%DIRECTORY_NAME%" == ".vs" exit /b 0
if /I "%DIRECTORY_NAME%" == "x64" exit /b 0

if not exist "%GRIDWHALE_ROOT%\%DIRECTORY_NAME%\" exit /b 0

echo [%DIRECTORY_NAME%]
for /F "usebackq delims=" %%F in (`git -c "safe.directory=%GRIDWHALE_ROOT%" -C "%GRIDWHALE_ROOT%" ls-files -- "%DIRECTORY_NAME%/"`) do call :updateFile "%%F" || exit /b 1

exit /b 0

:updateFile
set "RELATIVE_PATH=%~1"
set "RELATIVE_PATH=%RELATIVE_PATH:/=\%"
set "SOURCE_FILE=%GRIDWHALE_ROOT%\%RELATIVE_PATH%"
set "DESTINATION_FILE=%HEXARC_ROOT%\%RELATIVE_PATH%"

rem A tracked file may be deleted in the GridWhale working tree. Do not purge
rem the Hexarc copy; deletions should be reviewed separately.
if not exist "%SOURCE_FILE%" exit /b 0

if exist "%DESTINATION_FILE%" (
    fc /B "%SOURCE_FILE%" "%DESTINATION_FILE%" >nul 2>&1
    if not errorlevel 1 exit /b 0
)

echo   %RELATIVE_PATH%
set /A FILES_UPDATED+=1
if defined DRY_RUN exit /b 0

for %%I in ("%DESTINATION_FILE%") do if not exist "%%~dpI" mkdir "%%~dpI" >nul 2>&1
copy /B /Y "%SOURCE_FILE%" "%DESTINATION_FILE%" >nul
if errorlevel 1 (
    echo ERROR: Unable to copy "%RELATIVE_PATH%".
    exit /b 1
)

exit /b 0

:usage
echo Usage: %~nx0 [GridWhaleRoot] [/L]
echo.
echo   GridWhaleRoot  Optional path to the GridWhale repository.
echo   /L             List the files that would be updated without copying them.
exit /b 1
