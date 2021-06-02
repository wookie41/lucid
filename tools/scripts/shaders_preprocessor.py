from sys import argv
from os.path import join
import re

USAGE = "./preprocessor <base-shaders-dir> <processed-shaders-dir> <shader-to-process-path> <processed-shader-path>"

include_args_regex = re.compile("\"(\S+)\"")

def handle_include(base_shaders_dir, shader_path, shader_source: str, token_start: int, token_end: int) -> str:
    match = include_args_regex.search(shader_source[token_end:])
    if match is None:
        print("Failed to handle '#include' action in '%s:%d' - invalid syntax"  % (shader_path, token_start))
        return None
    shader_to_include_path = join(base_shaders_dir, match.group(1))
    try:
        with open(shader_to_include_path) as f:
            to_include = ''.join(f.readlines())
    except Exception as e:
        print("Failed to handle '#include' action in '%s:%d' - couldn't open %s :%s"  % (shader_path, token_start, shader_to_process_path, str(e)))
        return None

    new_source = shader_source[:token_start]
    new_source += to_include
    new_source += "\n"
    new_source += shader_source[token_end + match.span(0)[1]:]
    
    return new_source

#actions take the curent source of the shader and transform it in any way there like

ignored_directives = ['version', 'define']
preprocessor_actions = {'include': handle_include}
preprocessor_action_regex = re.compile("#([a-z]+)")

if len(argv) < 5:
    print(USAGE)
    exit(1)

base_shaders_dir = argv[1]
processed_shaders_dir = argv[2]
shader_to_process_path = argv[3]
processed_shader_path = argv[4]

shader_source = None
try:
    path = join(base_shaders_dir, shader_to_process_path)
    with open(path, 'rb') as f:
        shader_source = ''.join(map(lambda b: b.decode('UTF-8'), f.readlines()))
except Exception as e:
    print("Failed to open file %s: %s" % (path, str(e)))
    exit(0)

current_pos = 0
while True:
    match = preprocessor_action_regex.search(shader_source[current_pos:])
    if match is None:
        break
    
    action = match.group(1)
    token_start, token_end = match.span()

    try:
        action_handler = preprocessor_actions[action]
    except KeyError:
        current_pos += token_end
        if action not in ignored_directives:
            print("Unsupported action '#%s' in shader '%s:%d'" % (action, path, token_start))
        continue

    shader_source = action_handler(base_shaders_dir, path, shader_source, token_start + current_pos, token_end + current_pos)
    if shader_source is None:
        exit(1)

    current_pos = 0

out_path = join(processed_shaders_dir, processed_shader_path)
try:
    with open(out_path, 'wb+') as f:
        f.write(shader_source.encode('utf-8'))
except Exception as e:
    print("Failed to save the result to '%s': %s" % (out_path, str(e)))
    exit(1)