@echo off
echo ========================================
echo Vulkan Shader Compilation Script
echo ========================================
echo.

set GLSLC_PATH="F:/Vulkan/Bin/glslc.exe"

if not exist %GLSLC_PATH% (
    echo Error: glslc.exe not found at %GLSLC_PATH%
    echo Please check your Vulkan SDK installation.
    pause
    exit /b 1
)

echo Found glslc.exe at %GLSLC_PATH%
echo.

rem 编译所有顶点着色器 (.vert 文件)
echo Compiling vertex shaders...
for %%f in (*.vert) do (
    echo   Compiling %%f...
    %GLSLC_PATH% "%%f" -o "%%~nf_vert.spv"
    if errorlevel 1 (
        echo Error: Failed to compile %%f
        pause
        exit /b 1
    )
    echo   Compiled: %%~nf_vert.spv
)

echo.

rem 编译所有片段着色器 (.frag 文件)
echo Compiling fragment shaders...
for %%f in (*.frag) do (
    echo   Compiling %%f...
    %GLSLC_PATH% "%%f" -o "%%~nf_frag.spv"
    if errorlevel 1 (
        echo Error: Failed to compile %%f
        pause
        exit /b 1
    )
    echo   Compiled: %%~nf_frag.spv
)

echo.

rem 编译所有计算着色器 (.comp 文件)
echo Compiling compute shaders...
for %%f in (*.comp) do (
    echo   Compiling %%f...
    %GLSLC_PATH% "%%f" -o "%%~nf_comp.spv"
    if errorlevel 1 (
        echo Error: Failed to compile %%f
        pause
        exit /b 1
    )
    echo   Compiled: %%~nf_comp.spv
)

echo.
echo ========================================
echo All shaders compiled successfully!
echo ========================================
echo.

rem 显示生成的 SPIR-V 文件
echo Generated SPIR-V files:
dir *.spv

echo.
pause