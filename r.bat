@echo off
call c-release.bat FRENCH fr
call c-release.bat ENGLISH en
zip -9 -u -q 6pad-src.zip *.c *.h *.rc *.bat *.dll *.a