@ECHO OFF
SETLOCAL

set sdlincdir=sdlinc
set sdllibdir=sdllib

IF NOT DEFINED VSCMD_VER (
	ECHO You must run this within Visual Studio Native Tools Command Line
	EXIT /B 1
)

WHERE GNUMAKE.EXE >nul 2>nul
IF ERRORLEVEL 1 (
	ECHO GNUMAKE.EXE was not found
	REM You can compile GNU Make within Visual Studio Native Tools Command
	REM Line natively by executing build_w32.bat within the root folder of
	REM GNU Make source code.
	REM You can download GNU Make source code at
	REM   http://ftpmirror.gnu.org/make/
	EXIT /B 1
)

REM Extract SDL2 headers and pre-compiled libraries
MKDIR %sdlincdir%
MKDIR %sdllibdir%
EXPAND tools\sdl2_2-0-12_inc.cab -F:* %sdlincdir%
EXPAND tools\sdl2_2-0-12_%VSCMD_ARG_TGT_ARCH%.cab -F:* %sdllibdir%
if errorlevel 1 (
	ECHO Could not expand tools\sdl2_2-0-12_%VSCMD_ARG_TGT_ARCH%.cab 
	EXIT /B 1
)

SET CC="cl"
SET OBJEXT="obj"
SET RM="del"
SET EXEOUT="/Fe"
SET CFLAGS="/nologo /analyze /diagnostics:caret /O2 /MD /GF /Zo- /fp:fast /W1 /I%sdlincdir%"
set LDFLAGS="/link /SUBSYSTEM:WINDOWS /OUT:Peanut-SDL-%VSCMD_ARG_TGT_ARCH%.exe /LIBPATH:'%sdllibdir%' /LTCG"
set LDLIBS="SDL2main.lib SDL2-static.lib winmm.lib msimg32.lib version.lib imm32.lib setupapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib"

REM Options specific to 32-bit platforms
if "%VSCMD_ARG_TGT_ARCH%"=="x32" (
	REM Uncomment the following to use SSE instructions (since Pentium III).
	REM set CFLAGS=%CFLAGS% /arch:SSE

	REM Uncomment the following to support ReactOS and Windows XP.
	set CFLAGS=%CFLAGS% /Fdvc141.pdb
)

set ICON_FILE=icon.ico
set RES=meta\winres.res

@ECHO ON
GNUMAKE.EXE CC=%CC% OBJEXT=%OBJEXT% RM=%RM% EXEOUT=%EXEOUT% CFLAGS=%CFLAGS% LDFLAGS=%LDFLAGS% LDLIBS=%LDLIBS% ICON_FILE=%ICON_FILE% RES=%RES% %1
@ECHO OFF

ENDLOCAL
@ECHO ON
