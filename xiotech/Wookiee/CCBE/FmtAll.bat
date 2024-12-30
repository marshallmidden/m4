if %1. == . goto error

copy k:\release\bigfoot\%1\ccb\buildz\ccbrun.map .  /y

k:\release\bigfoot\%1\ccbe\fmtccbdumps.pl CALLSTACK callstack.corelis
k:\release\bigfoot\%1\ccbe\fmtccbdumps.pl SERIAL Serial.corelis
k:\release\bigfoot\%1\ccbe\fmtccbdumps.pl TRACE Trace.corelis

goto done

:error
@echo you need to specify a release. I.e.,   "%0 M501 <return>"


:done

