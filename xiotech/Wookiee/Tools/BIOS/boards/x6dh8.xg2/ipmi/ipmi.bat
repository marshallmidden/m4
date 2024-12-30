@ECHO OFF
ECHO Updating - IPMI...
FLASH -d dh8g2b12.mot
CALL RetCode.BAT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR

ECHO Configuring BMC...
SMCFG
CALL RetCode.BAT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR
GOTO END

:EXIT_ERROR
ECHO Error encountered during update

:END
ECHO IPMI update batch file finished
