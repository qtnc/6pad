@echo off
for %%i in (*.c) do gcc -c -s -O6 -Os -std=gnu99 %%i -w -o obj\%%~ni.o -D%1
windres res.rc -o obj\rc.o -D%1
gcc -s -O6 -Os -w obj\*.o -o 6pad-%2.exe -lpcre -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lwinmm -lluaex -lluajit -mthreads -mwindows -Wl,--gc-sections -Wl,--major-image-version,1 -Wl,--minor-image-version,0
zip -9 -u -q 6pad-%2.zip 6pad-%2.exe lua51.dll changelog.txt "6pad scripting documentation.chm"