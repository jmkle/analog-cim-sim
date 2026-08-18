/******************************************************************************
 * Copyright (C) 2026 Joel Klein                                              *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file        *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#include "mapping/mapping_registry.h"
#include "mapping/bnn_mapper/bnn_i.h"
#include "mapping/bnn_mapper/bnn_ii.h"
#include "mapping/bnn_mapper/bnn_iii.h"
#include "mapping/bnn_mapper/bnn_iv.h"
#include "mapping/bnn_mapper/bnn_v.h"
#include "mapping/bnn_mapper/bnn_vi.h"
#include "mapping/int_mapper/int_i.h"
#include "mapping/int_mapper/int_ii.h"
#include "mapping/int_mapper/int_iii.h"
#include "mapping/int_mapper/int_iv.h"
#include "mapping/int_mapper/int_v.h"
#include "mapping/mapper.h"
#include "mapping/tnn_mapper/tnn_i.h"
#include "mapping/tnn_mapper/tnn_ii.h"
#include "mapping/tnn_mapper/tnn_iii.h"
#include "mapping/tnn_mapper/tnn_iv.h"
#include "mapping/tnn_mapper/tnn_v.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>

namespace nq {
namespace {

/** Everything the registry knows about one mapping mode. */
struct RegistryEntry {
    const MappingProperties *props;      /**< How the mapping places a weight */
    std::unique_ptr<Mapper> (*create)(); /**< Builds the mapper of that mode */
};

template <typename T> std::unique_ptr<Mapper> make_mapper() {
    return std::make_unique<T>();
}

// One row per mode, in the order of the enums.
constexpr RegistryEntry REGISTRY[] = {
    {&MapperIntI::PROPERTIES_1XB, &make_mapper<MapperIntI>},
    {&MapperIntI::PROPERTIES_2XB, &make_mapper<MapperIntI>},
    {&MapperIntII::PROPERTIES, &make_mapper<MapperIntII>},
    {&MapperIntIII::PROPERTIES, &make_mapper<MapperIntIII>},
    {&MapperIntIV::PROPERTIES, &make_mapper<MapperIntIV>},
    {&MapperIntV::PROPERTIES, &make_mapper<MapperIntV>},
    {&MapperBnnI::PROPERTIES, &make_mapper<MapperBnnI>},
    {&MapperBnnII::PROPERTIES, &make_mapper<MapperBnnII>},
    {&MapperBnnIII::PROPERTIES, &make_mapper<MapperBnnIII>},
    {&MapperBnnIV::PROPERTIES, &make_mapper<MapperBnnIV>},
    {&MapperBnnV::PROPERTIES, &make_mapper<MapperBnnV>},
    {&MapperBnnVI::PROPERTIES, &make_mapper<MapperBnnVI>},
    {&MapperTnnI::PROPERTIES, &make_mapper<MapperTnnI>},
    {&MapperTnnII::PROPERTIES, &make_mapper<MapperTnnII>},
    {&MapperTnnIII::PROPERTIES, &make_mapper<MapperTnnIII>},
    {&MapperTnnIV::PROPERTIES, &make_mapper<MapperTnnIV>},
    {&MapperTnnV::PROPERTIES, &make_mapper<MapperTnnV>},
};

/** A mode without a row makes the table shorter than the enum, so the size ties
 * the two together and no entry can be left value initialized. */
static_assert(std::size(REGISTRY) ==
                  static_cast<size_t>(MappingMode::NUM_MODES),
              "The mapping registry does not have one row per MappingMode");

/** Whether every row sits at the index of the mode it describes. */
constexpr bool registry_is_ordered() {
    for (size_t i = 0; i < std::size(REGISTRY); ++i) {
        if (REGISTRY[i].props->mode != static_cast<MappingMode>(i)) {
            return false;
        }
    }
    return true;
}
static_assert(registry_is_ordered(),
              "The mapping registry rows are not in MappingMode order");

/** Row of a mode, aborts on NUM_MODES and on anything cast in from outside the
 * enum. */
const RegistryEntry &entry_of(MappingMode mode, const char *what) {
    const size_t index = static_cast<size_t>(mode);
    if (index >= std::size(REGISTRY)) {
        std::cerr << "No " << what << " for mapping mode " << index << "."
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return REGISTRY[index];
}

} // namespace

const MappingProperties &mapping_properties(MappingMode mode) {
    return *entry_of(mode, "properties").props;
}

std::optional<MappingMode> mode_from_name(const std::string &name) {
    for (const RegistryEntry &entry : REGISTRY) {
        if (name == entry.props->name) {
            return entry.props->mode;
        }
    }
    return {};
}

std::string m_mode_to_string(MappingMode mode) {
    const size_t index = static_cast<size_t>(mode);
    if (index >= std::size(REGISTRY)) {
        return "Unknown mode";
    }
    return REGISTRY[index].props->name;
}

std::unique_ptr<Mapper> create_mapper(MappingMode mode) {
    return entry_of(mode, "mapper").create();
}

} // namespace nq
