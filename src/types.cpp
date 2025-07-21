#define MOBILITYDUCK_EXTENSION_TYPES

#include "types.hpp"
#include "duckdb/common/extension_type_info.hpp"
#include <regex>
#include <string>

extern "C" {
    #include "meos/meos.h"
    #include "meos/meos_catalog.h"
}

namespace duckdb {


LogicalType GeoTypes::SPAN() {
    auto type = LogicalType::STRUCT({
        {"lower", LogicalType::BIGINT},
        {"upper", LogicalType::BIGINT},
        {"lower_inc",LogicalType::BOOLEAN},
        {"upper_inc", LogicalType::BOOLEAN},
        {"basetype", LogicalType::UTINYINT}
        
    });
    type.SetAlias("SPAN");
    return type;
}


inline Datum Int32GetDatum(int32_t value) {
    return (Datum)value;
}


inline void ExecuteSpanMake(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &lower_vec = args.data[0];
    auto &upper_vec = args.data[1];
    auto &lower_inc_vec = args.data[2];
    auto &upper_inc_vec = args.data[3];

    lower_vec.Flatten(count);
    upper_vec.Flatten(count);
    lower_inc_vec.Flatten(count);
    upper_inc_vec.Flatten(count);

    auto &children = StructVector::GetEntries(result);
    auto &lower_child = children[0];
    auto &upper_child = children[1];
    auto &lower_inc_child = children[2];
    auto &upper_inc_child = children[3];
    auto &basetype_child = children[4];

    for (idx_t i = 0; i < count; i++) {
        auto lower = lower_vec.GetValue(i).GetValue<int32_t>();
        auto upper = upper_vec.GetValue(i).GetValue<int32_t>();
        auto lower_inc = lower_inc_vec.GetValue(i).GetValue<bool>();
        auto upper_inc = upper_inc_vec.GetValue(i).GetValue<bool>();

        Span *span = span_make(
            Int32GetDatum(lower),
            Int32GetDatum(upper),
            lower_inc,
            upper_inc,
            T_INT4  // Cast to uint8_t
        );

        lower_child->SetValue(i, Value::BIGINT(span->lower));
        upper_child->SetValue(i, Value::BIGINT(span->upper));
        lower_inc_child->SetValue(i, Value::BOOLEAN(span->lower_inc));
        upper_inc_child->SetValue(i, Value::BOOLEAN(span->upper_inc));
        basetype_child->SetValue(i, Value::UTINYINT(span->basetype));
        
        free(span);
    }
    
    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}


inline void ExecuteSpanIn(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &input_vec = args.data[0];

    input_vec.Flatten(count);

    auto &children = StructVector::GetEntries(result);
    auto &lower_child = children[0];
    auto &upper_child = children[1];
    auto &lower_inc_child = children[2];
    auto &upper_inc_child = children[3];
    auto &basetype_child = children[4];

    std::regex pattern(R"(^([\[\(])\s*(-?\d+)\s*,\s*(-?\d+)\s*([\]\)])$)");

    for (idx_t i = 0; i < count; i++) {
        std::string input = input_vec.GetValue(i).ToString();
        std::smatch match;
        if (!std::regex_match(input, match, pattern)) {
            throw InvalidInputException("Invalid span format: " + input);
        }

        bool lower_inc = match[1].str() == "[";
        int32_t lower = std::stoll(match[2].str());
        int32_t upper = std::stoll(match[3].str());
        bool upper_inc = match[4].str() == "]";

        // Reuse span_make
        auto span = span_make(Int32GetDatum(lower), Int32GetDatum(upper),
                              lower_inc, upper_inc, T_INT4);

        lower_child->SetValue(i, Value::BIGINT(span->lower));
        upper_child->SetValue(i, Value::BIGINT(span->upper));
        lower_inc_child->SetValue(i, Value::BOOLEAN(span->lower_inc));
        upper_inc_child->SetValue(i, Value::BOOLEAN(span->upper_inc));
        basetype_child->SetValue(i, Value::UTINYINT(span->basetype));

        free(span);
    }

    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}

inline void ExecuteSpanOut(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &span_vec = args.data[0];
    
    for (idx_t i = 0; i < count; i++) {
        auto span_struct = span_vec.GetValue(i);
        auto &children = StructValue::GetChildren(span_struct);
        
        // Reconstruct the Span struct
        Span span;
        span.lower = children[0].GetValue<int64_t>();
        span.upper = children[1].GetValue<int64_t>();
        span.lower_inc = children[2].GetValue<bool>();
        span.upper_inc = children[3].GetValue<bool>();
        span.basetype = (meosType)children[4].GetValue<uint8_t>();
        
        // Use span_out to convert to string
        char *str = span_out(&span, 0);
        result.SetValue(i, Value(str));
        free(str);
    }
}


inline void ExecuteSpanInOut(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &input_vec = args.data[0];

    input_vec.Flatten(count);

    std::regex pattern(R"(^([\[\(])\s*(-?\d+)\s*,\s*(-?\d+)\s*([\]\)])$)");

    for (idx_t i = 0; i < count; i++) {
        std::string input = input_vec.GetValue(i).ToString();
        std::smatch match;
        if (!std::regex_match(input, match, pattern)) {
            throw InvalidInputException("Invalid span format: " + input);
        }

        bool lower_inc = match[1].str() == "[";
        int32_t lower = std::stoll(match[2].str());
        int32_t upper = std::stoll(match[3].str());
        bool upper_inc = match[4].str() == "]";

        // Create span using span_make (this will canonicalize)
        Span *span = span_make(Int32GetDatum(lower), Int32GetDatum(upper),
                              lower_inc, upper_inc, T_INT4);

        // Convert back to string using span_out
        char *str = span_out(span, 0);
        result.SetValue(i, Value(str));
        
        free(str);
        free(span);
    }

    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}

void GeoTypes::RegisterScalarFunctions(DatabaseInstance &instance) {

    // Add this to your RegisterScalarFunctions method
    auto span_make_function = ScalarFunction(
        "SPAN_MAKE", // name
        {LogicalType::BIGINT, LogicalType::BIGINT, LogicalType::BOOLEAN, LogicalType::BOOLEAN}, // arguments: lower, upper, lower_inc, upper_inc
        LogicalType::STRUCT({  // return type - you'd need to define a SPAN type
            {"lower", LogicalType::BIGINT},
            {"upper", LogicalType::BIGINT},
            {"lower_inc", LogicalType::BOOLEAN},
            {"upper_inc", LogicalType::BOOLEAN},
            {"basetype", LogicalType::UTINYINT}
        }),
        ExecuteSpanMake); // function implementation
    ExtensionUtil::RegisterFunction(instance, span_make_function);

    auto intspan_in_function = ScalarFunction(
        "SPAN_IN",
        {LogicalType::VARCHAR},
        GeoTypes::SPAN(),
        ExecuteSpanIn
    );
    ExtensionUtil::RegisterFunction(instance, intspan_in_function);

    auto span_out_function = ScalarFunction(
        "SPAN_OUT",
        {GeoTypes::SPAN()},
        LogicalType::VARCHAR,
        ExecuteSpanOut
    );
    ExtensionUtil::RegisterFunction(instance, span_out_function);

    // Combined function: string input -> canonicalized string output
    auto intspan_function = ScalarFunction(
        "INTSPAN",
        {LogicalType::VARCHAR},
        LogicalType::VARCHAR,  // Return VARCHAR instead of SPAN struct
        ExecuteSpanInOut
    );
    ExtensionUtil::RegisterFunction(instance, intspan_function);

}

void GeoTypes::RegisterTypes(DatabaseInstance &instance) {
    ExtensionUtil::RegisterType(instance, "SPAN", GeoTypes::SPAN());
}

} // namespace duckdb

#ifndef MOBILITYDUCK_EXTENSION_TYPES
#endif