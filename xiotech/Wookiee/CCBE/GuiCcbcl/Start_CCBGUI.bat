:: $Header$

:: starts the jar file

:: set environement
if defined JAVA_HOME goto envexist
set JAVA_HOME=K:\Tools\Java\jdk1.3.1_02\bin

:envexist
set path=%PATH%;%JAVA_HOME%

::run jar file
java -jar GuiCcbcl.jar

::
:: $Log$
:: Revision 1.1  2005/05/04 18:53:54  RysavyR
:: Initial revision
::
:: Revision 1.6  2002/02/21 22:12:05  HoltyB
:: made changes to allow to save window settings
:: added ccbcl.log view
::
:: Revision 1.5  2002/02/14 16:00:32  HoltyB
:: added compatability for nt 4.0 environement settings for JAVA_HOME
::
:: Revision 1.4  2002/02/14 14:07:02  HoltyB
:: added checking for JAVA_HOME Environement setting
::
:: Revision 1.3  2002/02/13 14:04:06  HoltyB
:: fixed environement path
::
:: Revision 1.2  2002/02/13 13:07:06  HoltyB
:: initial integration of GUICCBCL
::
::