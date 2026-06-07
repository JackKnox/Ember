#pragma once

#include "ember/core.h"

typedef u32 em_version;

#define EMBER_MAKE_VERSION(major, minor, patch) \
    ((((u32)(major)) << 22U) | (((u32)(minor)) << 12U) | ((u32)(patch)))

#define EMBER_VERSION_MAJOR(version) ((u32)(version) >> 22U)
#define EMBER_VERSION_MINOR(version) (((u32)(version) >> 12U) & 0x3FFU)
#define EMBER_VERSION_PATCH(version) ((u32)(version) & 0xFFFU)

#define EMBER_VERSIONS_COMPLIANT(version1, version2) (EMBER_VERSION_MAJOR(version1) == EMBER_VERSION_MAJOR(version2))