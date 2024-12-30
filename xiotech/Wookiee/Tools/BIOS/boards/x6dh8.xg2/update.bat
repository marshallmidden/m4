@ECHO OFF
REM *** Update the system BIOS
CD BIOS
CALL BIOS.BAT
CD ..
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR

:UPDATE_BMC
REM *** Update the BMC
CD IPMI
CALL IPMI.BAT
CD ..
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR

:EXIT_GOOD
ECHO System update complete - no errors
GOTO END

:EXIT_ERROR
ECHO Error %RETURN_CODE% encountered during update

:END
ECHO Update batch file finished
