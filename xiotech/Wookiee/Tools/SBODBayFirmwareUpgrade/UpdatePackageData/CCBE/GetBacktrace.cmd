# Get Timestamp, FW Version and R/G registers
memread -tbinary -f c:/temp/BtHdr.bin 0xFEEB4050 160
!BtHdrDec.pl c:\temp\BtHdr.bin
!echo.                                                                      > c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!echo Timestamp, FW Version and R/G registers:                             >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!\\RSTSTORE\Private\Tools\gnu\cat c:\temp\BtHdr.bin-out                    >> c:\BackTrace.txt

#  Get Trace Data
memread -tbinary -f c:/temp/trace.bin 0xFEEB45A0 20480 
!tracDec.pl c:\temp\trace.bin
!echo.                                                                     >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!echo Trace Data:                                                          >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!\\RSTSTORE\Private\Tools\gnu\cat c:\temp\trace.bin-out                    >> c:\BackTrace.txt

# Get Serial Port Data
memread -tbinary -f c:/temp/serial.txt 0xFEEB95A0 20480 
!echo.                                                                     >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!echo Serial Data:                                                         >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!\\RSTSTORE\Private\Tools\gnu\cat c:\temp\serial.txt                       >> c:\BackTrace.txt

# Get Callstack
memread -tbinary -f c:/temp/callstack.txt 0xFEEB4190 1024
!echo.                                                                     >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!echo Call Stack:                                                          >> c:\BackTrace.txt
!echo -------------------------------------------------------------------- >> c:\BackTrace.txt
!\\RSTSTORE\Private\Tools\gnu\cat c:\temp\callstack.txt                    >> c:\BackTrace.txt


!\\RSTSTORE\Private\Tools\gnu\cat c:\BackTrace.txt



# YOUR BACKTRACE DATA HAS BEEN SAVED TO: c:\BackTrace.txt
# BE SURE TO COPY IT OUT IF YOU NEED TO SAVE IT, AS IT
# WILL BE OVERWRITTEN THE NEXT TIME YOU RUN THIS SCRIPT!

quit
