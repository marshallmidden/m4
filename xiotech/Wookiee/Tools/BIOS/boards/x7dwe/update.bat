@ECHO OFF
REM *** Update the system BIOS
CALL BIOS.BAT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR

:EXIT_GOOD
ECHO System update complete - no errors
GOTO END

:EXIT_ERROR
ECHO Error %RETURN_CODE% encountered during update

:END
ECHO Update batch file finished
