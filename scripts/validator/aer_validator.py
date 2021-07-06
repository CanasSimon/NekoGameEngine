#!/usr/bin/env python3

import platform
import subprocess
import os
from enum import Enum
from pathlib import Path

if platform.system() == 'Windows':
    vulkan_path = os.getenv("VULKAN_SDK")
    program = '{}\\Bin\\glslangValidator.exe'.format(vulkan_path)
else:
    program = 'glslangValidator'


class ShaderType(Enum):
    MISC = 0
    VERT = 1
    FRAG = 2
    TESC = 3
    TESE = 4
    GEOM = 5
    COMP = 6


type_sizes = {
    'bool': 1,
    'int': 4,
    'uint': 4,
    'float': 4,
    'double': 8,
    'vec2': 8,
    'vec3': 12,
    'vec4': 16,
    'mat2': 16,
    'mat3': 36,
    'mat4': 64,
    'sampler2D': 0
}

type_to_enum = {
    'float': 1,
    'vec2': 2,
    'vec3': 3,
    'vec4': 4,
    'int': 5,
    'ivec2': 6,
    'ivec3': 7,
    'ivec4': 8,
    'uint': 9,
    'uvec2': 10,
    'uvec3': 11,
    'uvec4': 12,
    'mat2': 13,
    'mat3': 14,
    'mat4': 15,
}

u_types = {
    'sampler2D': 1,
    'sampler3D': 2,
}

stage_flags = {
    '.vert': 1,
    '.frag': 16,
    '.geom': 8,
    '.comp': 32,
    '.tese': 4,
    '.tesc': 2,
}


def removeprefix(extension, prefix) :
    if extension.startswith(prefix):
        return extension[len(prefix):]
    else:
        return extension[:]


def validate_aer_material(data_src, data_out, meta_content):
    global validator_exe, validator_path, data_binary_path
    data_binary_path = os.getenv("DATA_BIN_FOLDER")
    validator_path = os.getenv("VALIDATOR_FOLDER")
    validator_exe = os.getenv("VALIDATE_JSON_EXE")

    status = subprocess.run([validator_exe, data_src, validator_path + "aer_material_validator.json"])
    print("Return Code: {}".format(status.returncode))
    print("validator_exe: {}".format(validator_exe))

    if status.returncode != 0:
        exit(1)

    # with open(data_src, 'r') as mat_file:
    #     mat_content = json.load(mat_file)
    #     mat_keys = mat_content.keys()
    #     new_content = {}
    #     uniforms = []
    #     for key in mat_keys:
    #         # Putting texture uuid
    #         if "map_path" in key:
    #             key_id = key.replace('path', 'id')
    #             if key_id not in mat_keys:
    #                 id = get_texture_id(os.path.join(data_binary_path, mat_content[key]))
    #                 new_content[key_id] = id
    #         # Loading shader content to material
    #         if "shader_path" in key:
    #             shader_path = os.path.join(data_binary_path, mat_content[key])
    #             with open(shader_path, 'r') as shader_content:
    #                 new_key = key.replace('path', 'content')
    #                 new_content[new_key] = shader_content.read()
    #             shader_meta_path = shader_path+".meta"
    #             with open(shader_meta_path, 'r') as shader_meta_file:
    #                 shader_meta = json.load(shader_meta_file)
    #                 if 'uniforms' in shader_meta:
    #                     for uniform_obj in shader_meta['uniforms']:
    #                         insert = True
    #                         # Avoid duplicate uniform
    #                         for u in uniforms:
    #                             if u['name'] == uniform_obj['name']:
    #                                 insert = False
    #                                 break
    #                         if insert:
    #                             uniforms.append(uniform_obj)
    #     new_content['uniforms'] = uniforms
    #     mat_content.update(new_content)
    #
    # with open(data_out, 'w') as mat_file:
    #     json.dump(mat_content, mat_file, indent=4)


def validate_aer_shader(data_src, data_out, meta_content):
    global validator_exe, validator_path, data_binary_path
    data_binary_path = os.getenv("DATA_BIN_FOLDER")
    validator_path = os.getenv("VALIDATOR_FOLDER")
    validator_exe = os.getenv("VALIDATE_JSON_EXE")

    status = subprocess.run([validator_exe, data_src, validator_path + "aer_shader_validator.json"])
    if status.returncode != 0:
        exit(1)
