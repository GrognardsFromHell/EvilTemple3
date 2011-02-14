
@echo off

mkdir out\release\bin
mkdir out\debug\bin

xcopy /Y thirdparty\taoframework-2.1.0\bin\*.dll* out\release\bin
xcopy /Y thirdparty\taoframework-2.1.0\bin\*.dll* out\debug\bin

xcopy /Y thirdparty\glew-1.5.7\bin\*.dll out\release\bin
xcopy /Y thirdparty\glew-1.5.7\bin\*.dll out\debug\bin

xcopy /Y thirdparty\libavcodec-minimal\*.dll out\release\bin
xcopy /Y thirdparty\libavcodec-minimal\*.dll out\debug\bin

xcopy /Y thirdparty\ninject-2.0\*.dll out\release\bin
xcopy /Y thirdparty\ninject-2.0\*.dll out\debug\bin

cd launcher
qmake -spec win32-msvc2010 -tp vc
cd ..

cd engine
qmake -spec win32-msvc2010 -tp vc
cd ..

cd qtmono
qmake -spec win32-msvc2010 -tp vc
cd ..

rem TODO: Setup the project dependencies in the project files (patch program needed)

mklink /J out\release\data data
mklink /J out\debug\data data
