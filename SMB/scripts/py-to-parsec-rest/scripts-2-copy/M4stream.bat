@echo off

REM create stream data on a file.

set V=%1
shift
if [%1] == [] goto exitnovolume
cd \
%V%

set D=%1
shift
if [%1] == [] goto exitnodir
cd \%D%

set file=%1
shift
if [%1] == [] goto exitnostream
set stream=%1
shift
if [%1] == [] goto nocontent
set content=%1
:loop1
shift
if [%1] == [] goto gathered
set content=%content% %1
goto loop1

:exitnovolume
echo No volume given.
goto leave

:exitnodir
echo No Directory given.
goto leave

:exitnostream
echo No stream for file %file% given.
goto leave

:nocontent
echo file=%file% stream='%stream%' empty file
echo > %file%:%stream%
goto leave

:gathered
echo file=%file% stream='%stream%' content='%content%'
echo %content% > %file%:%stream%

:leave
