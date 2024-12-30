@ECHO OFF

SET RETURN_CODE=1

SET BIOS_FILE=BIOS.ROM
ECHO Updating - BIOS... %BIOS_FILE%

ECHO Using new flash utility...
PHLASH16 /mfg /mode=3 /BBL /c /s /x /exit /DMS:Xiotech %BIOS_FILE%
IF ERRORLEVEL 1 GOTO EXIT_ERROR
SET RETURN_CODE=0
GOTO END

:EXIT_ERROR
SET RETURN_CODE=1
ECHO Error encountered during update

:END
ECHO BIOS update batch file finished
