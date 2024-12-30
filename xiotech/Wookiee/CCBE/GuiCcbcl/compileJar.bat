:: $Header$

:: compile java files for guiccbcl and make jar file
:: javac and jar must be in your path
:: i.e. set path=%PATH%;C:\jdk1.3.1_02\bin

:: set environement
set path=%PATH%;K:\Tools\Java\jdk1.3.1_02\bin

::compile files
javac CCBCLguiSetup.java
if ERRORLEVEL 1 goto error
javac CCBCLguiFunction.java
if ERRORLEVEL 1 goto error
javac CCBCLguiCreateFunc.java
if ERRORLEVEL 1 goto error
javac CCBCLguiSysCallStdThread.java
if ERRORLEVEL 1 goto error
javac CCBCLguiSysCall.java
if ERRORLEVEL 1 goto error
javac CCBCLguiServer.java
if ERRORLEVEL 1 goto error
javac CCBCLguiFontChooser.java
if ERRORLEVEL 1 goto error
javac CCBCLguiFrame.java
if ERRORLEVEL 1 goto error
javac CCBCLguiMain.java
if ERRORLEVEL 1 goto error

:: create jar file
jar.exe cvfm GuiCcbcl.jar manifest.tmp *.class
if ERRORLEVEL 1 goto error

:: delete class files
del C*********************************.class

goto done

:error
echo error occured
pause
goto quit

:done
echo finished successfully
pause

:quit

::
:: $Log$
:: Revision 1.1  2005/05/04 18:53:54  RysavyR
:: Initial revision
::
:: Revision 1.8  2002/02/21 22:12:05  HoltyB
:: made changes to allow to save window settings
:: added ccbcl.log view
::
:: Revision 1.7  2002/02/19 16:31:15  HoltyB
:: Added many new features
:: AsyncServer
:: DebugConsole
::
:: Revision 1.6  2002/02/15 23:32:33  HoltyB
:: Major change to persistent connection to ccbcl.pl
::
:: Revision 1.5  2002/02/14 15:59:42  HoltyB
:: removed checking for JAVA_HOME Environement setting
:: JAVA_HOME is for runtime only
::
:: Revision 1.4  2002/02/14 14:07:02  HoltyB
:: added checking for JAVA_HOME Environement setting
::
:: Revision 1.3  2002/02/13 17:22:04  HoltyB
:: took out some nt 4.0 incompatabilities
::
:: Revision 1.2  2002/02/13 14:04:06  HoltyB
:: fixed environement path
::
:: Revision 1.1  2002/02/13 13:05:37  HoltyB
:: initial integration of GUICCBCL
::
::