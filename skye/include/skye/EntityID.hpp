#pragma once

#include <cstdint>

namespace skye {

using EntityID = uint32_t;

// 14 generation bits leave 18 index bits: one per entity EntityRegistry can hold.
inline constexpr unsigned ENTITY_GENERATION_BITS = 14;

}
