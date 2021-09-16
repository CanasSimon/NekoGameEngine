#pragma once
#include "math/hash.h"

namespace neko::vk
{
using ResourceHash = StringHash;

struct Texture
{
    enum Flags : unsigned
    {
        SMOOTH_TEXTURE     = 1u << 0u,
        MIPMAPS_TEXTURE    = 1u << 1u,
        CLAMP_WRAP         = 1u << 2u,
        REPEAT_WRAP        = 1u << 3u,
        MIRROR_REPEAT_WRAP = 1u << 4u,
        GAMMA_CORRECTION   = 1u << 5u,
        FLIP_Y             = 1u << 6u,
        HDR                = 1u << 7u,
        DEFAULT            = REPEAT_WRAP | SMOOTH_TEXTURE | MIPMAPS_TEXTURE
    };

    Vec2i size;
};
}
