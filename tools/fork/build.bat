@echo off
rem Release build with ThinLTO + the Ocarina of Time PGO profile, same flags as the official
rem windows-x86_64-pgo package. Usage: tools\fork\build.bat <output dir>   (e.g. C:\dev\lichii-build\out-perf)
rem Toolchain on this PC: VS 2022 Build Tools, LLVM (clang-cl), CMake, Ninja, Vulkan SDK, Qt 6.8.3 in C:\Qt.
if "%~1"=="" (echo usage: %~nx0 ^<output dir^> & exit /b 1)
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
for /d %%V in (C:\VulkanSDK\*) do set "VKSDK=%%V"
set "PATH=%VKSDK%\Bin;C:\Program Files\CMake\bin;C:\Program Files\LLVM\bin;%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;%PATH%"
cd /d "%~dp0..\.."
set "SRC=%CD%"
set "F=-DWIN32 -D_WINDOWS -DNOMINMAX -flto=thin -fprofile-use=%SRC:\=/%/lichii-windows.profdata -Wno-error=profile-instr-out-of-date -Wno-error=profile-instr-unprofiled -Wno-error=backend-plugin"
cmake -S . -B "%~1" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DYUZU_TESTS=OFF -DYUZU_CMD=ON -DYUZU_ROOM=OFF -DYUZU_ROOM_STANDALONE=OFF -DYUZU_USE_BUNDLED_QT=OFF -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_64 -DYUZU_USE_BUNDLED_SDL3=ON -DENABLE_UPDATE_CHECKER=ON -DLICHII_BUILD_PLATFORM=windows-x86_64-pgo -DUSE_DISCORD_PRESENCE=OFF -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DCMAKE_AR=llvm-lib "-DCMAKE_C_FLAGS=%F%" "-DCMAKE_CXX_FLAGS=-EHsc %F%" || exit /b 1
rem 12 jobs: ThinLTO links on 16 GB RAM.
cmake --build "%~1" --parallel 12
