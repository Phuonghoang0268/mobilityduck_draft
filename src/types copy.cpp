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



LogicalType GeoTypes::TINSTANT() {
    auto type = LogicalType::STRUCT({
        {"value", LogicalType::BIGINT},  // Using BIGINT for Datum for now
        {"temptype", LogicalType::UTINYINT},
        {"t", LogicalType::TIMESTAMP_TZ}});
    type.SetAlias("TINSTANT");
    return type;
}


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


inline Datum Int64GetDatum(int64_t value) {
    return (Datum)value;
}



inline void ExecuteTintMake(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &value_vec = args.data[0];
    auto &t_vec = args.data[1];

    value_vec.Flatten(count);
    t_vec.Flatten(count);

    auto &children = StructVector::GetEntries(result);
    auto &value_child = children[0];
    auto &temptype_child = children[1];
    auto &t_child = children[2];

    for (idx_t i = 0; i < count; i++) {
        auto value = value_vec.GetValue(i).GetValue<int64_t>();
        auto t = t_vec.GetValue(i).GetValue<timestamp_tz_t>();
        TInstant *inst = tinstant_make((Datum)value, T_TINT, (TimestampTz)t.value);
        value_child->SetValue(i, Value::BIGINT((int64_t)inst->value));
        temptype_child->SetValue(i, Value::UTINYINT(inst->temptype));
        t_child->SetValue(i, Value::TIMESTAMPTZ(timestamp_tz_t(inst->t)));
        free(inst);
    }
    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}

// SQL function implementations
inline void ExecuteTInstantMake(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &value_vec = args.data[0];
    auto &temptype_vec = args.data[1];
    auto &t_vec = args.data[2];

    value_vec.Flatten(count);
    temptype_vec.Flatten(count);
    t_vec.Flatten(count);

    auto &children = StructVector::GetEntries(result);
    auto &value_child = children[0];
    auto &temptype_child = children[1];
    auto &t_child = children[2];

    for (idx_t i = 0; i < count; i++) {
        auto value = value_vec.GetValue(i).GetValue<int64_t>();
        auto temptype = temptype_vec.GetValue(i).GetValue<uint8_t>();
        auto t = t_vec.GetValue(i).GetValue<timestamp_tz_t>();

        TInstant *inst = tinstant_make((Datum)value, temptype, (TimestampTz)t.value);
        value_child->SetValue(i, Value::BIGINT((int64_t)inst->value));
        temptype_child->SetValue(i, Value::UTINYINT(inst->temptype));
        t_child->SetValue(i, Value::TIMESTAMPTZ(timestamp_tz_t(inst->t)));
        free(inst);
    }
    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}

inline void ExecuteTInstantValue(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &tinstant_vec = args.data[0];
    tinstant_vec.Flatten(count);

    auto &children = StructVector::GetEntries(tinstant_vec);
    auto &value_child = children[0];
    auto &temptype_child = children[1];
    auto &t_child = children[2];

    for (idx_t i = 0; i < count; i++) {
        TInstant inst;
        inst.value = value_child->GetValue(i).GetValue<int64_t>();
        inst.temptype = temptype_child->GetValue(i).GetValue<uint8_t>();
        inst.t = t_child->GetValue(i).GetValue<timestamp_tz_t>().value;
        Datum val = tinstant_value(&inst);
        result.SetValue(i, Value::BIGINT((int64_t)val));
    }
    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
}

inline void ExecuteTInstantToString(DataChunk &args, ExpressionState &state, Vector &result) {
    auto count = args.size();
    auto &tinstant_vec = args.data[0];
    tinstant_vec.Flatten(count);

    auto &children = StructVector::GetEntries(tinstant_vec);
    auto &value_child = children[0];
    auto &temptype_child = children[1];
    auto &t_child = children[2];

    for (idx_t i = 0; i < count; i++) {
        TInstant inst;
        inst.value = value_child->GetValue(i).GetValue<int64_t>();
        inst.temptype = temptype_child->GetValue(i).GetValue<uint8_t>();
        inst.t = t_child->GetValue(i).GetValue<timestamp_tz_t>().value;
        char *str = tinstant_to_string(&inst, 0, value_out_int);
        result.SetValue(i, Value(str));
        free(str);
    }
    if (count == 1) {
        result.SetVectorType(VectorType::CONSTANT_VECTOR);
    }
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
        auto lower = lower_vec.GetValue(i).GetValue<int64_t>();
        auto upper = upper_vec.GetValue(i).GetValue<int64_t>();
        auto lower_inc = lower_inc_vec.GetValue(i).GetValue<bool>();
        auto upper_inc = upper_inc_vec.GetValue(i).GetValue<bool>();

        Span *span = span_make(
            Int64GetDatum(lower),
            Int64GetDatum(upper),
            lower_inc,
            upper_inc,
            T_INT8  // Cast to uint8_t
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
        int64_t lower = std::stoll(match[2].str());
        int64_t upper = std::stoll(match[3].str());
        bool upper_inc = match[4].str() == "]";

        // Reuse span_make
        auto span = span_make(Int64GetDatum(lower), Int64GetDatum(upper),
                              lower_inc, upper_inc, T_INT8);

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



void GeoTypes::RegisterScalarFunctions(DatabaseInstance &instance) {
    // Register TInstant functions
    auto tinstant_make_function = ScalarFunction(
        "TINSTANT_MAKE", // name
        {LogicalType::BIGINT, LogicalType::UTINYINT, LogicalType::TIMESTAMP_TZ}, // arguments
        GeoTypes::TINSTANT(), // return type
        ExecuteTInstantMake); // function
    ExtensionUtil::RegisterFunction(instance, tinstant_make_function);

    auto tinstant_value_function = ScalarFunction(
        "TINSTANT_VALUE", // name
        {GeoTypes::TINSTANT()}, // arguments
        LogicalType::BIGINT, // return type
        ExecuteTInstantValue); // function
    ExtensionUtil::RegisterFunction(instance, tinstant_value_function);

    auto tinstant_to_string_function = ScalarFunction(
        "TINSTANT_TO_STRING", // name
        {GeoTypes::TINSTANT()}, // arguments
        LogicalType::VARCHAR, // return type
        ExecuteTInstantToString); // function
    ExtensionUtil::RegisterFunction(instance, tinstant_to_string_function);

    // Register TINT function
    auto tint_make_function = ScalarFunction(
        "TINT", // name
        {LogicalType::BIGINT, LogicalType::TIMESTAMP_TZ}, // arguments
        GeoTypes::TINSTANT(), // return type
        ExecuteTintMake); // function
    ExtensionUtil::RegisterFunction(instance, tint_make_function);

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
        "INTSPAN",
        {LogicalType::VARCHAR},
        GeoTypes::SPAN(),
        ExecuteSpanIn
    );
    ExtensionUtil::RegisterFunction(instance, intspan_in_function);


}

void GeoTypes::RegisterTypes(DatabaseInstance &instance) {
    ExtensionUtil::RegisterType(instance, "TINSTANT", GeoTypes::TINSTANT());
}

} // namespace duckdb

#ifndef MOBILITYDUCK_EXTENSION_TYPES
#endif