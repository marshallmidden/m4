echo off

if x%3==x goto usage
if x%2==x goto usage
if x%1==x goto usage

echo.
echo ISE Add Datapac version 3.0
echo.
tclsh.exe ise_add_datapac_win -D -C -d %1 -v 8 -i %2 -p %3
if errorlevel 1 goto error
if errorlevel 2 goto error2
if errorlevel 2 goto error3
echo.
echo ISE Add DataPac have completed successfully.
goto end
:usage
echo.
echo   Usage: %0 dsc_id ip_address com_port
echo.
echo     dsc id - DSC Identifier.
echo ip address - IP address for MRC-1 of the ISE.
echo              Note: IP address for MRC-2 will be same but
echo                    last digit will be one greater.
echo   com port - COM port used to connect to the ISE.
echo.
echo Example: %0 10855 172.16.1.2 1
echo.
:error
echo.
echo Failed to Add ISE DataPac.
:error2
echo.
echo Data already exists on both Datapacs.  If you are trying to reconfigure the ISE, please run ISE_SETUP.bat.
:error3
echo.
echo Datapac add have FAILED.
:end

