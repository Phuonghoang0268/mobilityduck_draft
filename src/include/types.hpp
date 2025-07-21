#pragma once


#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

namespace duckdb {

// Define the MEOS temptype for tint (should match meosType enum)
// #ifndef T_TINT
// #define T_TINT 6
// #endif

enum class TemporalGeometryType: uint8_t {
    TGEOMPOINT = 0,
    TGEOGPOINT
};

struct TemporalGeometryTypes {
    static LogicalType TGEOMPOINT();
    static LogicalType TGEOGPOINT();

    static string ToString(TemporalGeometryType type) {
        switch (type) {
        case TemporalGeometryType::TGEOMPOINT:
            return "TGEOMPOINT";
        case TemporalGeometryType::TGEOGPOINT:
            return "TGEOGPOINT";
        default:
            return StringUtil::Format("Unknown (%d)", static_cast<int>(type));
        }
    }
};


class ExtensionLoader;

struct GeoTypes {
    static LogicalType TINSTANT();
    static LogicalType SPAN();
    static void RegisterTypes(DatabaseInstance &instance);
    static void RegisterScalarFunctions(DatabaseInstance &instance);
};

} // namespace duckdb