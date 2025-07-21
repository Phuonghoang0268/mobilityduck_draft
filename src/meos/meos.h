#ifndef MEOS_H
#define MEOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#ifndef int16
typedef signed short int16;
#endif
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Enumeration that defines the built-in and temporal types used in
 * MobilityDB.
 */
typedef enum
{
  T_UNKNOWN        = 0,   /**< unknown type */
  T_BOOL           = 1,   /**< boolean type */
  T_DATE           = 2,   /**< date type */
  T_DATESET        = 5,   /**< date set type */
  T_DATESPAN       = 6,   /**< date span type */
  T_DATESPANSET    = 7,   /**< date span set type */
  T_FLOAT8         = 11,  /**< float8 type */
  T_FLOATSET       = 12,  /**< float8 set type */
  T_FLOATSPAN      = 13,  /**< float8 span type */
  T_FLOATSPANSET   = 14,  /**< float8 span set type */
  T_INT4           = 15,  /**< int4 type */
  T_INTSET         = 18,  /**< int4 set type */
  T_INTSPAN        = 19,  /**< int4 span type */
  T_INTSPANSET     = 20,  /**< int4 span set type */
  T_INT8           = 21,  /**< int8 type */
  T_BIGINTSET      = 22,  /**< int8 set type */
  T_BIGINTSPAN     = 23,  /**< int8 span type */
  T_BIGINTSPANSET  = 24,  /**< int8 span set type */
  T_TFLOAT         = 33,  /**< temporal float type */
  T_TIMESTAMPTZ    = 34,  /**< timestamp with time zone type */
  T_TINT           = 35,  /**< temporal integer type */
  T_TSTZSET        = 38,  /**< timestamptz set type */
  T_TSTZSPAN       = 39,  /**< timestamptz span type */
  T_TSTZSPANSET    = 40,  /**< timestamptz span set type */
} meosType;

#define NO_MEOS_TYPES 63


typedef struct
{
  meosType spantype;    /**< Enum value of the span type */
  meosType basetype;    /**< Enum value of the base type */
} spantype_catalog_struct;

/**
 * Structure to represent the spanset type cache array.
 */
typedef struct
{
  meosType spansettype;    /**< Enum value of the span type */
  meosType spantype;       /**< Enum value of the base type */
} spansettype_catalog_struct;

/*****************************************************************************/


/* Type conversion functions */

extern meosType spantype_basetype(meosType type);
extern meosType basetype_spantype(meosType type);

/* Catalog functions */
extern const char *meostype_name(meosType type);
extern bool meos_basetype(meosType type);
extern bool span_basetype(meosType type);
extern bool span_canon_basetype(meosType type);
extern bool span_type(meosType type);



typedef uint8_t uint8;
typedef int32_t int32;
typedef int16_t int16;

typedef struct
{ 
    int32_t vl_len_;
    uint8_t temptype;
    uint8_t subtype;
    int16_t flags;
} Temporal;

extern Temporal *tpoint_cumulative_length(const Temporal *temp);

// --- Minimal TInstant port for DuckDB integration ---
// For now, use int64_t for TimestampTz and uintptr_t for Datum
typedef int64_t TimestampTz;
typedef uintptr_t Datum;

typedef struct {
    int32_t vl_len_;
    uint8_t temptype;
    uint8_t subtype;
    int16_t flags;
    Datum value;
    TimestampTz t;
} TInstant;

typedef struct
{
  uint8 spantype;       /**< span type */
  uint8 basetype;       /**< span basetype */
  bool lower_inc;       /**< lower bound is inclusive (vs exclusive) */
  bool upper_inc;       /**< upper bound is inclusive (vs exclusive) */
  char padding[4];      /**< Not used */
  Datum lower;          /**< lower bound value */
  Datum upper;          /**< upper bound value */
} Span;

/**
 * Structure to represent span sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint8 spansettype;    /**< Span set type */
  uint8 spantype;       /**< Span type */
  uint8 basetype;       /**< Span basetype */
  char padding;         /**< Not used */
  int32 count;          /**< Number of elements */
  int32 maxcount;       /**< Maximum number of elements */
  Span span;            /**< Bounding span */
  Span elems[1];        /**< Beginning of variable-length data */
} SpanSet;


extern bool p_obracket(const char **str);
extern bool p_cbracket(const char **str);
extern bool p_oparen(const char **str);
extern void p_whitespace(const char **str);


extern bool basetype_in(const char *str, meosType type, bool end, Datum *result);
extern char *basetype_out(Datum value, meosType type, int maxdd);

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef long int int64;

#define UNUSED          __attribute__((unused))

#define PG_INT32_MAX	(0x7FFFFFFF)
#define PG_INT64_MAX	INT64CONST(0x7FFFFFFFFFFFFFFF)


TInstant *tinstant_make(Datum value, uint8_t temptype, TimestampTz t);
Datum tinstant_value(const TInstant *inst);
char *value_out_int(Datum value, int maxdd);
char *tinstant_to_string(const TInstant *inst, int maxdd, char *(*value_out)(Datum value, int maxdd));

Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,   meosType basetype);

char *span_out(const Span *s, int maxdd);





// SpanSet * tinstant_timestamptz(const TInstant *inst);

#ifdef __cplusplus
}
#endif

#endif // MEOS_H