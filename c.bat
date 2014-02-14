@echo off
for %%i in (*.c) do gcc -c -g -std=gnu99 %%i -w -o obj\%%~ni.o -DFRENCH
windres res.rc -o obj\rc.o -DFRENCH
gcc -g -w obj\*.o -o test.exe -lpcre-utf8 -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lluaex -lluajit -mthreads