#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/FileSchemeStructures.h"

namespace QSpace::IO::SchemeFactory {
ReadScheme createScheme_v2_3(Visualize::EntityType type, FileFormat format);
ReadScheme createScheme_v2(Visualize::EntityType type, FileFormat format);
} // namespace QSpace::IO::SchemeFactory