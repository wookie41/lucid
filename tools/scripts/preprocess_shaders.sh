#!/bin/bash
BASE_SHADERS_DIR="shaders/glsl/base"
PROCESSED_SHADERS_DIR="shaders/glsl"

echo "Pre-processing shaders..."
echo

for shader_path in {$BASE_SHADERS_DIR/*.vert,$BASE_SHADERS_DIR/*.frag,$BASE_SHADERS_DIR/*.geom}; do
    shader_path=${shader_path//"$BASE_SHADERS_DIR/"/}
    echo "Processing $shader_path...";
    python3 tools/scripts/shaders_preprocessor.py $BASE_SHADERS_DIR $PROCESSED_SHADERS_DIR $shader_path $shader_path
    if [ $? -eq 0 ]; then
        echo "--- OK ---";
    else
        echo "--- ERROR ---";
    fi

    echo  
done

echo "-- DONE ---"