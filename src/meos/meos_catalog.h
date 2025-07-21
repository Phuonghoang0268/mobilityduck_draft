
// #ifndef __MEOS_CATALOG_H__
// #define __MEOS_CATALOG_H__

// /* C */
// #include <stdbool.h>
// /* PostgreSQL */
// #ifndef int16
// typedef signed short int16;
// #endif
// /* MEOS */
// #include <meos.h>

// /*****************************************************************************
//  * Data structures
//  *****************************************************************************/

// /**
//  * @brief Enumeration that defines the built-in and temporal types used in
//  * MobilityDB.
//  */
// typedef enum
// {
//   T_UNKNOWN        = 0,   /**< unknown type */
//   T_BOOL           = 1,   /**< boolean type */
//   T_DATE           = 2,   /**< date type */
//   T_DATESET        = 5,   /**< date set type */
//   T_DATESPAN       = 6,   /**< date span type */
//   T_DATESPANSET    = 7,   /**< date span set type */
//   T_FLOAT8         = 11,  /**< float8 type */
//   T_FLOATSET       = 12,  /**< float8 set type */
//   T_FLOATSPAN      = 13,  /**< float8 span type */
//   T_FLOATSPANSET   = 14,  /**< float8 span set type */
//   T_INT4           = 15,  /**< int4 type */
//   T_INTSET         = 18,  /**< int4 set type */
//   T_INTSPAN        = 19,  /**< int4 span type */
//   T_INTSPANSET     = 20,  /**< int4 span set type */
//   T_INT8           = 21,  /**< int8 type */
//   T_BIGINTSET      = 22,  /**< int8 set type */
//   T_BIGINTSPAN     = 23,  /**< int8 span type */
//   T_BIGINTSPANSET  = 24,  /**< int8 span set type */
//   T_TFLOAT         = 33,  /**< temporal float type */
//   T_TIMESTAMPTZ    = 34,  /**< timestamp with time zone type */
//   T_TINT           = 35,  /**< temporal integer type */
//   T_TSTZSET        = 38,  /**< timestamptz set type */
//   T_TSTZSPAN       = 39,  /**< timestamptz span type */
//   T_TSTZSPANSET    = 40,  /**< timestamptz span set type */
// } meosType;

// #define NO_MEOS_TYPES 63


// typedef struct
// {
//   meosType spantype;    /**< Enum value of the span type */
//   meosType basetype;    /**< Enum value of the base type */
// } spantype_catalog_struct;

// /**
//  * Structure to represent the spanset type cache array.
//  */
// typedef struct
// {
//   meosType spansettype;    /**< Enum value of the span type */
//   meosType spantype;       /**< Enum value of the base type */
// } spansettype_catalog_struct;

// /*****************************************************************************/


// /* Type conversion functions */

// extern meosType spantype_basetype(meosType type);
// extern meosType basetype_spantype(meosType type);

// /* Catalog functions */

// extern bool meos_basetype(meosType type);

// extern bool span_basetype(meosType type);
// extern bool span_canon_basetype(meosType type);
// extern bool span_type(meosType type);


// /*****************************************************************************/

// #endif /* __MEOS_CATALOG_H__ */

