echo off

set SOLUTIONNAME=firmware
set PROJECTNAME=MethaneDetector

set PROJECTSPATH=D:\ZSL\github\MethaneDetector
set AVRDUDE="D:\ZSL\electro\bin\avrdude\avrdude.exe"
set AVRDUDE_ASP="D:\ZSL\electro\bin\avrdude_usbasp\avrdude.exe"

set HEX=%PROJECTSPATH%\%SOLUTIONNAME%\%PROJECTNAME%\Debug\%PROJECTNAME%.hex"

set CHIPMODEL=t2313
rem -F

set OPTIONS=-p %CHIPMODEL% -c 2ftbb -P ft0 -B 19200
rem set OPTIONS=-p %CHIPMODEL% -c usbasp
rem -F

rem %AVRDUDE_ASP% %OPTIONS% -U flash:w:%HEX%:a
rem 
%AVRDUDE% %OPTIONS% -U flash:w:%HEX%:a

rem верификация
rem %AVRDUDE% %OPTIONS% -U flash:v:%HEX%:a

rem получить фьюзы с контролера
rem %AVRDUDE% %OPTIONS% -U hfuse:r:high.txt:s -U lfuse:r:low.txt:s -U efuse:r:ext.txt:s

rem записать фьюзы на контроллер
rem    - кварц + делитель
rem %AVRDUDE% %OPTIONS% -U lfuse:w:0x7f:m -U hfuse:w:0xdf:m

rem записать дефолтные фьюзы на контроллер
rem    - дефолт
rem %AVRDUDE% %OPTIONS% -U lfuse:w:0x62:m -U hfuse:w:0xdf:m

rem %AVRDUDE% -h
rem %AVRDUDE% -c 2ftbb -t

pause