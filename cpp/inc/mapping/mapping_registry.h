/******************************************************************************
 * Copyright (C) 2026 Joel Klein                                              *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file        *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#ifndef MAPPING_REGISTRY_H
#define MAPPING_REGISTRY_H

#include <memory>
#include <optional>
#include <string>

#include "helper/definitions.h"
#include "mapping/mapping_properties.h"

namespace nq {

class Mapper;

/** Properties of a mapping mode.
 *
 * Resolves a mode to the constant of the mapper that implements it, for the
 * code that has to know how a mapping behaves before a mapper exists.
 *
 * A new mode needs an enum in definitions.h, a PROPERTIES constant on its
 * mapper and a row in the REGISTRY array in mapping_registry.cpp.
 */
const MappingProperties &mapping_properties(MappingMode mode);

/** Mode a config name refers to, empty if no mode carries that name. */
std::optional<MappingMode> mode_from_name(const std::string &name);

/** Config name of a mapping mode. */
std::string m_mode_to_string(MappingMode mode);

/** Mapper that implements a mode, built from the current configuration. */
std::unique_ptr<Mapper> create_mapper(MappingMode mode);

} // namespace nq

#endif
