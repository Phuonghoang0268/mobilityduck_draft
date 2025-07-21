// /**
//  * @file
//  * @brief Create a cache of metadata information about MEOS types in global
//  * constant arrays
//  */

// #include "meos.h"

// /* C */
// #include <assert.h>
// #include <limits.h>
// /* PostgreSQL */
// #if NPOINT
//   #include "npoint/tnpoint.h"
// #endif
// #if CBUFFER
//   #include <meos_cbuffer.h>
// #endif

// /*****************************************************************************
//  * Global constants
//  *****************************************************************************/

// /**
//  * @brief Global constant array containing the type names corresponding to the
//  * enumeration meosType defined in file `meos_catalog.h`
//  */
// static const char *MEOS_TYPE_NAMES[] =
// {
//   [T_UNKNOWN] = "unknown",
//   [T_BOOL] = "bool",
//   [T_DATE] = "date",
//   [T_DATESET] = "dateset",
//   [T_DATESPAN] = "datespan",
//   [T_DATESPANSET] = "datespanset",
//   [T_FLOAT8] = "float8",
//   [T_FLOATSET] = "floatset",
//   [T_FLOATSPAN] = "floatspan",
//   [T_FLOATSPANSET] = "floatspanset",
//   [T_INT4] = "int4",
//   [T_INTSET] = "intset",
//   [T_INTSPAN] = "intspan",
//   [T_INTSPANSET] = "intspanset",
//   [T_INT8] = "int8",
//   [T_BIGINTSET] = "bigintset",
//   [T_BIGINTSPAN] = "bigintspan",
//   [T_BIGINTSPANSET] = "bigintspanset",
//   [T_TFLOAT] = "tfloat",
//   [T_TIMESTAMPTZ] = "timestamptz",
//   [T_TINT] = "tint",
//   [T_TSTZSET] = "tstzset",
//   [T_TSTZSPAN] = "tstzspan",
//   [T_TSTZSPANSET] = "tstzspanset",
  
// };

// #define TEMPSUBTYPE_STR_MAXLEN 12


// /*****************************************************************************/

// /**
//  * @brief Global constant array that keeps type information for the defined
//  * span types
//  */
// static const spantype_catalog_struct MEOS_SPANTYPE_CATALOG[] =
// {
//   /* spantype       basetype */
//   {T_BIGINTSPAN,    T_INT8},
//   {T_TSTZSPAN,      T_TIMESTAMPTZ},
// };

// /**
//  * @brief Global constant array that keeps type information for the defined
//  * span set types
//  */
// static const spansettype_catalog_struct MEOS_SPANSETTYPE_CATALOG[] =
// {
//   /* spansettype    spantype */
//   {T_INTSPANSET,    T_INTSPAN},
//   {T_BIGINTSPANSET, T_BIGINTSPAN},
//   {T_TSTZSPANSET,   T_TSTZSPAN},
// };

// /*****************************************************************************
//  * Catalog functions
//  *****************************************************************************/

// /**
//  * @brief Return the base type from the span type
//  */
// meosType
// spantype_basetype(meosType type)
// {
//   int n = sizeof(MEOS_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
//   for (int i = 0; i < n; i++)
//   {
//     if (MEOS_SPANTYPE_CATALOG[i].spantype == type)
//       return MEOS_SPANTYPE_CATALOG[i].basetype;
//   }
 
//   return T_UNKNOWN;
// }

// /**
//  * @brief Return the span type of a base type
//  */
// meosType
// basetype_spantype(meosType type)
// {
//   int n = sizeof(MEOS_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
//   for (int i = 0; i < n; i++)
//   {
//     if (MEOS_SPANTYPE_CATALOG[i].basetype == type)
//       return MEOS_SPANTYPE_CATALOG[i].spantype;
//   }
//   return T_UNKNOWN;
// }


// bool
// span_canon_basetype(meosType type)
// {
//   if (type == T_DATE || type == T_INT4 || type == T_INT8)
//     return true;
//   return false;
// }