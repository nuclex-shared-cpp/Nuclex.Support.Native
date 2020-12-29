::!%windir%\System32\cmd.exe
@ECHO OFF

SET processorCount=%NUMBER_OF_PROCESSORS%

FOR %%m IN (Debug Release) DO (

	REM Delete existing CMake intermediate directory
	REM
	REM CMake doesn't support anything but a full rebuild unless you manually
	REM enter each and every source file into your CMakeLists.txt. See the
	REM documentation on file(GLOB_RECURSE ...) for that turd.
	REM
	IF EXIST obj\cmake-%%m rd /s /q obj\cmake-%%m

	REM Let CMake build a Makefile (that's the default)
	cmake -B obj\cmake-%%m -DCMAKE_BUILD_TYPE=%%m

	REM Compile the binary
	cmake --build obj\cmake-%%m --config %%m --parallel %processorCount%

	REM Put build artifacts in ./bin/windows-msvc14.1-amd64-release/ or similar
	cmake --install obj\cmake-%%m --config %%m

)
