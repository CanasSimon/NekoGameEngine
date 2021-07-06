#!/usr/bin/env python3
import subprocess
from os import environ
from pathlib import Path


def validate_vkshader(data_src, data_out, meta_content):
    global glslang_exe
    glslang_exe = environ.get("GLSLANG_VALIDATOR_EXE")
    if glslang_exe is None:
        sys.stderr.write("Could not find glslangValidator executable\n")
        exit(1)

    base_file = data_out.replace('.glsl', '')
    path = Path(base_file)
    stage = path.suffix.replace('.', '')
    data_out = base_file + '.spv'
    status = subprocess.run([glslang_exe, '--target-env', 'spirv1.4', '-V', data_src, '-o', data_out])
    if status.returncode != 0:
        exit(1)

