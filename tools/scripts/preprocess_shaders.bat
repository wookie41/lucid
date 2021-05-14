@ECHO OFF
SET BASE_SHADERS_DIR=shaders\glsl\base
SET PROCESSED_SHADERS_DIR=shaders\glsl

@ECHO Pre-processing shaders...

FOR %%f IN (%BASE_SHADERS_DIR%\*.frag, %BASE_SHADERS_DIR%\*.vert, BASE_SHADERS_DIR\*.geom) DO (
    @ECHO Processing %%~nxf
    CALL python3 tools/scripts/shaders_preprocessor.py %BASE_SHADERS_DIR% %PROCESSED_SHADERS_DIR% %%~nxf %%~nxf
    IF %ERRORLEVEL% NEQ 0 (
        @ECHO --- ERROR ---
    ) ELSE (
        @ECHO --- OK ---
    )
)
