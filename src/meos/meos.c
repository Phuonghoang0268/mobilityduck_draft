#include "meos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "meos_catalog.h"
#include <assert.h>

#define Assert(condition)	((void)true)

static inline Datum Int32GetDatum(int32_t value) {
    return (Datum)value;
}

static inline int32_t DatumGetInt32(Datum datum) {
    return (int32_t)datum;
}

static inline Datum Int64GetDatum(int64_t value) {
    return (Datum)value;
}

static inline int64_t DatumGetInt64(Datum datum) {
    return (int64_t)datum;
}

static inline Datum DateADTGetDatum(int32_t value) {
    return (Datum)value;
}

static inline int32_t DatumGetDateADT(Datum datum) {
    return (int32_t)datum;
}



TInstant *tinstant_make(Datum value, uint8_t temptype, TimestampTz t) {
    TInstant *inst = (TInstant *)malloc(sizeof(TInstant));
    if (!inst) return NULL;
    inst->vl_len_ = sizeof(TInstant);
    inst->temptype = temptype;
    inst->subtype = 0; // For now, set to 0
    inst->flags = 0;   // For now, set to 0
    inst->value = value;
    inst->t = t;
    return inst;
}

Datum tinstant_value(const TInstant *inst) {
    if (!inst) return (Datum)0;
    return inst->value;
}



// Example value_out function for integers
char *value_out_int(Datum value, int maxdd) {
    (void)maxdd;
    char *buf = (char *)malloc(32);
    snprintf(buf, 32, "%ld", (long)value);
    return buf;
}

char *tinstant_to_string(const TInstant *inst, int maxdd, char *(*value_out)(Datum value, int maxdd)) {
    if (!inst || !value_out) return NULL;
    char *valstr = value_out(inst->value, maxdd);
    char *buf = (char *)malloc(128 + strlen(valstr));
    snprintf(buf, 128 + strlen(valstr), "%s@%" PRId64, valstr, inst->t);
    free(valstr);
    return buf;
}

// Datum comparison function
int datum_cmp(Datum a, Datum b, meosType basetype)
{
  switch (basetype)
  {
    case T_INT4:
    {
      int32_t val_a = DatumGetInt32(a);
      int32_t val_b = DatumGetInt32(b);
      if (val_a < val_b) return -1;
      if (val_a > val_b) return 1;
      return 0;
    }
    case T_INT8:
    {
      int64_t val_a = DatumGetInt64(a);
      int64_t val_b = DatumGetInt64(b);
      if (val_a < val_b) return -1;
      if (val_a > val_b) return 1;
      return 0;
    }
    case T_DATE:
    {
      int32_t val_a = DatumGetDateADT(a);
      int32_t val_b = DatumGetDateADT(b);
      if (val_a < val_b) return -1;
      if (val_a > val_b) return 1;
      return 0;
    }
    default:
      // For unsupported types, treat as equal
      return 0;
  }
}

Datum
span_incr_bound(Datum lower, meosType basetype)
{
  Datum result;
  switch (basetype)
  {
    case T_INT4:
      result = Int32GetDatum(DatumGetInt32(lower) + (int32) 1);
      break;
    case T_INT8:
      result = Int64GetDatum(DatumGetInt64(lower) + (int32) 1);
      break;
    case T_DATE:
      result = DateADTGetDatum(DatumGetDateADT(lower) + 1);
      break;
    default:
      result = lower;
  }
  return result;
}

void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  meosType basetype, meosType spantype, Span *s)
{
  assert(s); assert(basetype_spantype(basetype) == spantype);
  /* Canonicalize */
  if (span_canon_basetype(basetype))
  {
    if (! lower_inc)
    {
      lower = span_incr_bound(lower, basetype);
      lower_inc = true;
    }
    if (upper_inc)
    {
      upper = span_incr_bound(upper, basetype);
      upper_inc = false;
    }
  }

  int cmp = datum_cmp(lower, upper, basetype);
  
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(s, 0, sizeof(Span));
  /* Fill in the span */
  s->lower = lower;
  s->upper = upper;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  s->spantype = spantype;
  s->basetype = basetype;
  return;
}

Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, 
  meosType basetype)
{
  Span *s = (Span *)malloc(sizeof(Span));
  meosType spantype = basetype_spantype(basetype);
  span_set(lower, upper, lower_inc, upper_inc, basetype, spantype, s);
  return s;
}

#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if CBUFFER
  #include <meos_cbuffer.h>
#endif

/*****************************************************************************
 * Global constants
 *****************************************************************************/

/**
 * @brief Global constant array containing the type names corresponding to the
 * enumeration meosType defined in file `meos_catalog.h`
 */
static const char *MEOS_TYPE_NAMES[] =
{
  [T_UNKNOWN] = "unknown",
  [T_BOOL] = "bool",
  [T_DATE] = "date",
  [T_DATESET] = "dateset",
  [T_DATESPAN] = "datespan",
  [T_DATESPANSET] = "datespanset",
  [T_FLOAT8] = "float8",
  [T_FLOATSET] = "floatset",
  [T_FLOATSPAN] = "floatspan",
  [T_FLOATSPANSET] = "floatspanset",
  [T_INT4] = "int4",
  [T_INTSET] = "intset",
  [T_INTSPAN] = "intspan",
  [T_INTSPANSET] = "intspanset",
  [T_INT8] = "int8",
  [T_BIGINTSET] = "bigintset",
  [T_BIGINTSPAN] = "bigintspan",
  [T_BIGINTSPANSET] = "bigintspanset",
  [T_TFLOAT] = "tfloat",
  [T_TIMESTAMPTZ] = "timestamptz",
  [T_TINT] = "tint",
  [T_TSTZSET] = "tstzset",
  [T_TSTZSPAN] = "tstzspan",
  [T_TSTZSPANSET] = "tstzspanset",
  
};

#define TEMPSUBTYPE_STR_MAXLEN 12


/*****************************************************************************/

/**
 * @brief Global constant array that keeps type information for the defined
 * span types
 */
static const spantype_catalog_struct MEOS_SPANTYPE_CATALOG[] =
{
  /* spantype       basetype */
  {T_BIGINTSPAN,    T_INT8},
  {T_TSTZSPAN,      T_TIMESTAMPTZ},
};

/**
 * @brief Global constant array that keeps type information for the defined
 * span set types
 */
static const spansettype_catalog_struct MEOS_SPANSETTYPE_CATALOG[] =
{
  /* spansettype    spantype */
  {T_INTSPANSET,    T_INTSPAN},
  {T_BIGINTSPANSET, T_BIGINTSPAN},
  {T_TSTZSPANSET,   T_TSTZSPAN},
};

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/**
 * @brief Return the base type from the span type
 */
meosType
spantype_basetype(meosType type)
{
  int n = sizeof(MEOS_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (MEOS_SPANTYPE_CATALOG[i].spantype == type)
      return MEOS_SPANTYPE_CATALOG[i].basetype;
  }
 
  return T_UNKNOWN;
}

/**
 * @brief Return the span type of a base type
 */
meosType
basetype_spantype(meosType type)
{
  int n = sizeof(MEOS_SPANTYPE_CATALOG) / sizeof(spantype_catalog_struct);
  for (int i = 0; i < n; i++)
  {
    if (MEOS_SPANTYPE_CATALOG[i].basetype == type)
      return MEOS_SPANTYPE_CATALOG[i].spantype;
  }
  return T_UNKNOWN;
}


bool
span_canon_basetype(meosType type)
{
  if (type == T_DATE || type == T_INT4 || type == T_INT8)
    return true;
  return false;
}


// const char *
// meostype_name(meosType type)
// {
//   return MEOS_TYPE_NAMES[type];
// }

// void
// p_whitespace(const char **str)
// {
//   while (**str == ' ' || **str == '\n' || **str == '\r' || **str == '\t')
//     *str += 1;
//   return;
// }


// bool
// p_obracket(const char **str)
// {
//   p_whitespace(str);
//   if (**str == '[')
//   {
//     *str += 1;
//     return true;
//   }
//   return false;
// }

// bool
// p_oparen(const char **str)
// {
//   p_whitespace(str);
//   if (**str == '(')
//   {
//     *str += 1;
//     return true;
//   }
//   return false;
// }


// bool
// bound_parse(const char **str, meosType basetype, Datum *result)
// {
//   p_whitespace(str);
//   int pos = 0;
//   while ((*str)[pos] != ',' && (*str)[pos] != ']' &&
//     (*str)[pos] != '}' && (*str)[pos] != ')' && (*str)[pos] != '\0')
//     pos++;
//   char *str1 = palloc(sizeof(char) * (pos + 1));
//   strncpy(str1, *str, pos);
//   str1[pos] = '\0';
//   bool success = basetype_in(str1, basetype, false, result);
//   pfree(str1);
//   if (! success)
//     return false;
//   *str += pos;
//   return true;
// }


// Span *
// span_in(const char *str, meosType spantype)
// {
//   assert(str);
//   Span result;
//   if (! span_parse(&str, spantype, true, &result))
//     return NULL;
//   return span_copy(&result);
// }

// bool span_parse(const char **str, meosType spantype, bool end, Span *span) {
//   const char *type_str = meostype_name(spantype);
//   meosType basetype = spantype_basetype(spantype);
//   Datum lower, upper;
//   bool lower_inc, upper_inc;

//   if ((lower_inc = p_obracket(str)) || p_oparen(str)) {
//     if (!bound_parse(str, basetype, &lower) || !p_comma(str) || !bound_parse(str, basetype, &upper)) {
//       return false;
//     }

//     if ((upper_inc = p_cbracket(str)) || p_cparen(str)) {
//       if (end && !ensure_end_input(str, type_str)) return false;
//       if (span) span_set(lower, upper, lower_inc, upper_inc, basetype, spantype, span);
//       return true;
//     }
//   }

//   return false;
// }
static char *
unquote(char *str)
{
  /* Save the initial pointer */
  char *result = str;
  char *last = str;
  while (*str != '\0')
  {
    if (*str != '"')
      *last++ = *str;
    str++;
  }
  *last = '\0';
  return result;
}

typedef unsigned long int uint64;
typedef unsigned int uint32;
#define UINT64CONST(x) (x##UL)


static const char DIGIT_TABLE[200] =
"00" "01" "02" "03" "04" "05" "06" "07" "08" "09"
"10" "11" "12" "13" "14" "15" "16" "17" "18" "19"
"20" "21" "22" "23" "24" "25" "26" "27" "28" "29"
"30" "31" "32" "33" "34" "35" "36" "37" "38" "39"
"40" "41" "42" "43" "44" "45" "46" "47" "48" "49"
"50" "51" "52" "53" "54" "55" "56" "57" "58" "59"
"60" "61" "62" "63" "64" "65" "66" "67" "68" "69"
"70" "71" "72" "73" "74" "75" "76" "77" "78" "79"
"80" "81" "82" "83" "84" "85" "86" "87" "88" "89"
"90" "91" "92" "93" "94" "95" "96" "97" "98" "99";


const uint8 pg_leftmost_one_pos[256] = {
	0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};


static inline int
pg_leftmost_one_pos32(uint32 word)
{
#ifdef HAVE__BUILTIN_CLZ
	Assert(word != 0);

	return 31 - __builtin_clz(word);
#else
	int			shift = 32 - 8;

	Assert(word != 0);

	while ((word >> shift) == 0)
		shift -= 8;

	return shift + pg_leftmost_one_pos[(word >> shift) & 255];
#endif							/* HAVE__BUILTIN_CLZ */
}


static inline int
decimalLength32(const uint32 v)
{
  int      t;
  static const uint32 PowersOfTen[] = {
    1, 10, 100,
    1000, 10000, 100000,
    1000000, 10000000, 100000000,
    1000000000
  };

  /*
   * Compute base-10 logarithm by dividing the base-2 logarithm by a
   * good-enough approximation of the base-2 logarithm of 10
   */
  t = (pg_leftmost_one_pos32(v) + 1) * 1233 / 4096;
  return t + (v >= PowersOfTen[t]);
}




pg_ultoa_n(uint32 value, char *a)
{
  int      olength,
        i = 0;

  /* Degenerate case */
  if (value == 0)
  {
    *a = '0';
    return 1;
  }

  olength = decimalLength32(value);

  /* Compute the result string. */
  while (value >= 10000)
  {
    const uint32 c = value - 10000 * (value / 10000);
    const uint32 c0 = (c % 100) << 1;
    const uint32 c1 = (c / 100) << 1;

    char     *pos = a + olength - i;

    value /= 10000;

    memcpy(pos - 2, DIGIT_TABLE + c0, 2);
    memcpy(pos - 4, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (value >= 100)
  {
    const uint32 c = (value % 100) << 1;

    char     *pos = a + olength - i;

    value /= 100;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (value >= 10)
  {
    const uint32 c = value << 1;

    char     *pos = a + olength - i;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
  }
  else
  {
    *a = (char) ('0' + value);
  }

  return olength;
}



int
pg_ltoa(int32 value, char *a)
{
  uint32    uvalue = (uint32) value;
  int      len = 0;

  if (value < 0)
  {
    uvalue = (uint32) 0 - uvalue;
    a[len++] = '-';
  }
  len += pg_ultoa_n(uvalue, a + len);
  a[len] = '\0';
  return len;
}




#define MAXINT4LEN 12
char *
int4_out(int32 val)
{
  char *result = malloc (MAXINT4LEN);  /* sign, 10 digits, '\0' */
  pg_ltoa(val, result);
  return result;
}



char *
basetype_out(Datum value, meosType type, int maxdd)
{
  assert(meos_basetype(type)); assert(maxdd >= 0);
  switch (type)
  {
    case T_INT4:
      return int4_out(DatumGetInt32(value));
    // case T_INT8:
    //   return int8_out(DatumGetInt64(value));
  }
}



char *
span_out(const Span *s, int maxdd)
{
  assert(s);

  char *lower = unquote(basetype_out(s->lower, s->basetype, maxdd));
  char *upper = unquote(basetype_out(s->upper, s->basetype, maxdd));
  char open = s->lower_inc ? (char) '[' : (char) '(';
  char close = s->upper_inc ? (char) ']' : (char) ')';
  size_t size = strlen(lower) + strlen(upper) + 5;
  char *result = (char*)malloc(size);
  snprintf(result, size, "%c%s, %s%c", open, lower, upper, close);
  free(lower); free(upper);
  return result;
}