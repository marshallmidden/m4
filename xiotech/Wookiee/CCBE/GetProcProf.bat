@echo off

:: %1 => ops/sec
:: %2 => Base directory path to BuildBE and BuildFE (i.e. C:\Projects\Bigfoot\Proc)
:: %3 => controller 1
:: %4 => controller 2 (optional)


:: One controller or two?
if "%1" == "" goto help
if "%2" == "" goto help
if "%3" == "" goto help
if "%4" == "" goto c1
  set NUM=2
goto c2
:c1
  set NUM=1
:c2

:: Start the profiling on FE/BE on both systems
if %NUM% == 2 goto c3
  ccbcl.pl %3 -e modebits -S -pPROC 8 8
goto c4
:c3
  ccbcl.pl %3 %4 -e 0; modebits -S -pPROC 8 8; 1; modebits -S -pPROC 8 8
:c4

:: Sleep 90 seconds (should be done in 60)
echo Sleeping 90 seconds...
k:\tools\gnu\sleep.exe 90

:: Suck the data down 
if %NUM% == 2 goto c5
  ccbcl.pl %3 -e fidread -tbinary -fbedata1 820; fidread -tbinary -ffedata1 564
goto c6
:c5
  ccbcl.pl %3 %4 -e 0; fidread -tbinary -fbedata1 820; fidread -tbinary -ffedata1 564; 1; fidread -tbinary -fbedata2 820; fidread -tbinary -ffedata2 564
:c6

:: decode the data
profdecProc.pl -l %2/BuildBE/berun.map bedata1 %1
del bedata1.out
rename bedata1-out bedata1.out

profdecProc.pl -l %2/BuildFE/ferun.map fedata1 %1
del fedata1.out
rename fedata1-out fedata1.out

if %NUM% == 1 goto c7
  profdecProc.pl -l %2/BuildBE/berun.map bedata2 %1
  del bedata2.out
  rename bedata2-out bedata2.out

  profdecProc.pl -l %2/BuildFE/ferun.map fedata2 %1
  del fedata2.out
  rename fedata2-out fedata2.out
:c7

echo Your results are found in [bf]edata[12].out

goto end

:help
echo.
echo Usage: GetProcProf.bat ops/sec base-dir-path IP1 [IP2]
echo. 
echo 'ops/sec'       - the ops per second that are currently running to the controllers
echo 'base-dir-path' - the 'Proc' directory you built in: C:\Projects\Bigfoot\Proc
echo 'IP1/IP2'       - the IP addresses of the controllers. Only 1 address is
echo                   required if running the test on a 1 way. 
echo.
goto end

:end
