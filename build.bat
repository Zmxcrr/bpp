@echo off
setlocal EnableDelayedExpansion

set "RED="
set "GREEN="
set "YELLOW="
set "BLUE="
set "NC="

set "INFO=[INFO]"
set "WARN=[WARN]"
set "ERR=[ERROR]"
set "NOTE=[NOTE]"

echo %INFO% C++23 Interpreter Build Script
echo %INFO% ==============================

set "BUILD_TYPE=Release"
set "BUILD_EXAMPLES=ON"
set "BUILD_TESTS=OFF"
set "CLEAN=0"
set "INSTALL=0"
set "VERBOSE=0"

:parse_args
if "%~1"=="" goto after_args
if "%~1"=="--debug"       (set "BUILD_TYPE=Debug"     & shift & goto parse_args)
if "%~1"=="--examples"    (set "BUILD_EXAMPLES=ON"    & shift & goto parse_args)
if "%~1"=="--no-examples" (set "BUILD_EXAMPLES=OFF"   & shift & goto parse_args)
if "%~1"=="--tests"       (set "BUILD_TESTS=ON"       & shift & goto parse_args)
if "%~1"=="--clean"       (set "CLEAN=1"              & shift & goto parse_args)
if "%~1"=="--install"     (set "INSTALL=1"            & shift & goto parse_args)
if "%~1"=="--verbose"     (set "VERBOSE=1"            & shift & goto parse_args)
if "%~1"=="--help"  (goto :show_help)
if "%~1"=="-h"      (goto :show_help)

echo %ERR% Unknown option: %~1
exit /b 1

:show_help
echo Usage: %~nx0 [options]
echo   --debug         Build in Debug mode ^(default: Release^)
echo   --examples      Build examples ^(default: ON^)
echo   --no-examples   Don't build examples
echo   --tests         Build tests ^(default: OFF^)
echo   --clean         Clean build directory first
echo   --install       Install after building
echo   --verbose       Verbose build output
echo   --help, -h      Show this help message
echo.
echo C++23 Requirements:
echo   - GCC 11+ or Clang 12+ or MSVC 2022+
echo   - CMake 3.20+
exit /b 0

:after_args

if not exist "CMakeLists.txt" (
  echo %ERR% CMakeLists.txt not found. Run from the project root.
  exit /b 1
)

echo %NOTE% Make sure a C++23-capable toolchain is active in this shell.

if "%CLEAN%"=="1" (
  echo %INFO% Cleaning build directory...
  if exist build rmdir /s /q build
)

if not exist build mkdir build
cd build

echo %INFO% Configuring C++23 project...
echo %INFO% Build type: %BUILD_TYPE%
echo %INFO% Examples: %BUILD_EXAMPLES%
echo %INFO% Tests: %BUILD_TESTS%

set "CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_EXAMPLES=%BUILD_EXAMPLES% -DBUILD_TESTS=%BUILD_TESTS% -DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_STANDARD_REQUIRED=ON"
if "%VERBOSE%"=="1" set "CMAK E_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON"

cmake .. %CMAKE_ARGS%
if errorlevel 1 (
  echo %ERR% CMake configuration failed!
  exit /b 1
)

echo %INFO% Building project with C++23...
set "JOBS=%NUMBER_OF_PROCESSORS%"
if "%JOBS%"=="" set "JOBS=4"

if "%VERBOSE%"=="1" (
  cmake --build . --parallel %JOBS% --verbose
) else (
  cmake --build . --parallel %JOBS%
)
if errorlevel 1 (
  echo %ERR% Build failed!
  exit /b 1
)

echo %INFO% Build completed successfully!

if /I "%BUILD_TESTS%"=="ON" (
  echo %INFO% Running tests...
  ctest --output-on-failure
  if errorlevel 1 (
    echo %ERR% Tests failed!
    exit /b 1
  )
)

if "%INSTALL%"=="1" (
  echo %INFO% Installing...
  cmake --install .
  if errorlevel 1 (
    echo %ERR% Install failed!
    exit /b 1
  )
)

echo %INFO% All done!
echo.
echo %INFO% Build artifacts:
echo   Library: build\lib\
if /I "%BUILD_EXAMPLES%"=="ON" echo   Examples: build\examples\
if /I "%BUILD_TESTS%"=="ON"    echo   Tests:    build\tests\
echo.
endlocal