@ECHO OFF

REM *** Call the SMBIOS/DMI board identification program (in PATH)
A: > nul
CD \ > nul
SET BOARD_NAME=UNKNOWN
BoardID
IF ERRORLEVEL 5 GOTO EXIT_ERROR
IF ERRORLEVEL 4 GOTO X7DWE
IF ERRORLEVEL 3 GOTO EXIT_ERROR
IF ERRORLEVEL 2 GOTO X6DH8_XG2
GOTO EXIT_ERROR

:X7DWE
SET BOARD_NAME=X7DWE
GOTO DO_BOARD

:X6DH8_XG2
SET BOARD_NAME=X6DH8.XG2
GOTO DO_BOARD

:DO_BOARD
IF EXIST \%BOARD_NAME%\UPDATE.BAT GOTO DO_UPDATE
ECHO !! Unknown board, %BOARD_TYPE% !!
GOTO EXIT_ERROR

:DO_UPDATE
REM *** %BOARD_NAME%
ECHO Flash %BOARD_NAME%
CD \%BOARD_NAME%
CALL UPDATE.BAT
CD \
IF %RETURN_CODE%==0 GOTO EXIT_GOOD
GOTO EXIT_ERROR

:EXIT_GOOD
ECHO **********************************************************************
ECHO ** System update finished - successful                              **
ECHO ** You may now eject the CD and ignore any resulting error message  **
ECHO **********************************************************************
:WAIT
REM Wait forever
PAUSE
GOTO WAIT

:EXIT_ERROR
ECHO Error during update, BOARD_TYPE=%BOARD_TYPE%, BOARD_NAME=%BOARD_NAME%

:END
PAUSE
GOTO END
