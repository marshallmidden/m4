@ECHO OFF
SET BIOS_FILE=BIOS.ROM
ECHO Updating - BIOS... %BIOS_FILE%

CHKFLASH %BIOS_FILE%
CALL RetCode.BAT
IF %RETURN_CODE% == 0 GOTO NEW_FLASH_UTILITY
IF %RETURN_CODE% == 1 GOTO OLD_FLASH_UTILITY
GOTO EXIT_ERROR

:NEW_FLASH_UTILITY
ECHO Using new flash utility...
PHLASH16 /p /s /c /mode=3 /exit %BIOS_FILE%
CALL RetCode.BAT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR
GOTO CMOS

:OLD_FLASH_UTILITY
ECHO Using old flash utility...
PHLASH /p /s /cz /mode=3 /exit %BIOS_FILE%
CALL RetCode.BAT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR
GOTO CMOS

:CMOS
ECHO Programming the CMOS settings...
SYMCMOS -D
SYMCMOS -UCMOS.SET
CALL RetCode.BAT
IF %RETURN_CODE%==11 GOTO REBOOT
IF NOT %RETURN_CODE%==0 GOTO EXIT_ERROR
GOTO END

:REBOOT
ECHO **!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**
ECHO ** Leave the CD in place and press the reset button or     !**
ECHO ** press control-alt-del to complete the update process.   !**
ECHO **!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**
PAUSE
GOTO REBOOT

:EXIT_ERROR
ECHO Error %RETURN_CODE% encountered during update

:END
ECHO BIOS update batch file finished
