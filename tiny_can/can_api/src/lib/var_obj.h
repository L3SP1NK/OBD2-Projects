#ifndef __VAR_OBJ__
#define __VAR_OBJ__

#include "mhs_obj.h"

#ifdef __cplusplus
  extern "C" {
#endif
/*
#define ERR_VAR_NOT_FOUND        -1    // Variable nicht gefunden
#define ERR_VAR_GET_PROTECT      -2    // Lesen der Variable nicht erlaubt
#define ERR_VAR_GET_PUFFER_SIZE  -3    // Lesepuffer für Variable zu klein
#define ERR_VAR_SET_PROTECT      -4    // Schreiben der Variable nicht erlaubt
#define ERR_VAR_SET_SIZE         -5    // Zu schreibender Wert zu groß
#define ERR_VAR_SET_MIN          -6    // Min Wert unterschritten
#define ERR_VAR_SET_MAX          -7    // Max Wert überschritten
#define ERR_ACCESS_DENIED        -8    // Zugriff verweigert
#define ERR_VAR_WRONG_TYPE       -9
#define ERR_FATAL_INTERNEL_ERROR -10
*/

#define MHS_VAL_FLAG_READ   0x01
#define MHS_VAL_FLAG_WRITE  0x02

#define VT_BYTE         0x01
#define VT_UBYTE        0x02
#define VT_WORD         0x03
#define VT_UWORD        0x04
#define VT_LONG         0x05
#define VT_ULONG        0x06

#define VT_BYTE_ARRAY   0x07
#define VT_UBYTE_ARRAY  0x08
#define VT_WORD_ARRAY   0x09
#define VT_UWORD_ARRAY  0x0A
#define VT_LONG_ARRAY   0x0B
#define VT_ULONG_ARRAY  0x0C

#define VT_BYTE_RANGE_ARRAY   0x0D
#define VT_UBYTE_RANGE_ARRAY  0x0E
#define VT_WORD_RANGE_ARRAY   0x0F
#define VT_UWORD_RANGE_ARRAY  0x10
#define VT_LONG_RANGE_ARRAY   0x11
#define VT_ULONG_RANGE_ARRAY  0x12

#define VT_BYTE_FIFO         0x21
#define VT_UBYTE_FIFO        0x22
#define VT_WORD_FIFO         0x23
#define VT_UWORD_FIFO        0x24
#define VT_LONG_FIFO         0x25
#define VT_ULONG_FIFO        0x26         

#define VT_HBYTE  0x40
#define VT_HWORD  0x41
#define VT_HLONG  0x42

#define VT_STREAM    0x80
#define VT_STRING    0x81
#define VT_POINTER   0x82
#define VT_REVISION  0x83
#define VT_DATE      0x84

#define VT_ANY       0xFF

#define VF_AUTO_CREATE  0x01
#define VF_NO_EVENT     0x02
#define VF_DEF          0x10  // <*> geändert von 0x80
#define VF_SET_CHANGE   0x80 // <*> neu

union TValue
  {
  int8_t S8;
  uint8_t U8;
  int16_t S16;  // signed short S16;
  uint16_t U16; // unsigned short U16;
  int32_t S32;  // signed long S32;
  uint32_t U32; // uint32_t U32;
  char *Str;
  void *Ptr;
  };

typedef struct _TByteRange TByteRange;

struct _TByteRange
  {
  int8_t Start;
  int8_t End;
  };

typedef struct _TUByteRange TUByteRange;

struct _TUByteRange
  {
  uint8_t Start;
  uint8_t End;
  };

typedef struct _TWordRange TWordRange;

struct _TWordRange
  {
  int16_t Start;
  int16_t End;
  };

typedef struct _TRange TUWordRange;

struct _TUWordRange
  {
  uint16_t Start;
  uint16_t End;
  };

typedef struct _TLongRange TLongRange;

struct _TLongRange
  {
  int32_t Start;
  int32_t End;
  };

typedef struct _TULongRange TULongRange;

struct _TULongRange
  {
  uint32_t Start;
  uint32_t End;
  };


typedef struct _TObjValue TObjValue;

struct _TObjValue
  {
  TMhsObj Obj;
  union TValue Value;
  int32_t Type;
  int32_t Size;
  int32_t MaxSize;
  int32_t Access;
  uint32_t Flags;
  void *Default;  
  uint32_t Status;
  };


struct TValueDescription
  {
  const char *Name;
  const char *Alias;
  int32_t Type;
  uint32_t Flags;
  int32_t MaxSize;
  int32_t Access;
  const void *Default;
  };


typedef void(*TObjValueCB)(TObjValue *obj, void *user_data);


TObjValue *mhs_value_create_unlocked(const char *name, int32_t type, int32_t max_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
TObjValue *mhs_value_create(const char *name, int32_t type, int32_t max_size, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_values_create_from_list(TMhsObjContext *context, const struct TValueDescription *list) ATTRIBUTE_INTERNAL;

void mhs_value_set_default(TObjValue *value) ATTRIBUTE_INTERNAL;
void mhs_values_set_default(TMhsObjContext *context) ATTRIBUTE_INTERNAL;
void mhs_values_set_all_default(void) ATTRIBUTE_INTERNAL;

int32_t mhs_value_set_as_byte(const char *name, int8_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ubyte(const char *name, uint8_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_word(const char *name, int16_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_uword(const char *name, uint16_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_long(const char *name, int32_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ulong(const char *name, uint32_t value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ptr(const char *name, void *value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_set_as_byte_array(const char *name, int8_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ubyte_array(const char *name, uint8_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_word_array(const char *name, int16_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_uword_array(const char *name, uint16_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_long_array(const char *name, int32_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ulong_array(const char *name, uint32_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_set_as_byte_rarray(const char *name, TByteRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ubyte_rarray(const char *name, TUByteRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_word_rarray(const char *name, TWordRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_uword_rarray(const char *name, TUWordRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_long_rarray(const char *name, TLongRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_as_ulong_rarray(const char *name, TULongRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_set_as_string(const char *name, const char *value, int32_t access, int32_t flags, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int8_t mhs_value_get_as_byte(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
uint8_t mhs_value_get_as_ubyte(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int16_t mhs_value_get_as_word(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
uint16_t mhs_value_get_as_uword(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_long(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
uint32_t mhs_value_get_as_ulong(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ulong_ex(const char *name, int32_t access, TMhsObjContext *context, uint32_t *value) ATTRIBUTE_INTERNAL;

void *mhs_value_get_as_ptr(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
uint32_t mhs_value_get_as_unsigned(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_get_as_byte_array(int8_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ubyte_array(uint8_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_word_array(int16_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_uword_array(uint16_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_long_array(int32_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ulong_array(uint32_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_get_as_byte_array_dup(int8_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ubyte_array_dup(uint8_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_word_array_dup(int16_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_uword_array_dup(uint16_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_long_array_dup(int32_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ulong_array_dup(uint32_t **array, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_get_as_byte_rarray(TByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ubyte_rarray(TUByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_word_rarray(TWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_uword_rarray(TUWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_long_rarray(TLongRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ulong_rarray(TULongRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_get_as_byte_rarray_dup(TByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ubyte_rarray_dup(TUByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_word_rarray_dup(TWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_uword_rarray_dup(TUWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_long_rarray_dup(TLongRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
int32_t mhs_value_get_as_ulong_rarray_dup(TULongRange **rarray, const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

char *mhs_value_get_as_string(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;
char *mhs_value_get_as_string_dup(const char *name, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_value_set_as_value_unlocked(TObjValue *obj, int32_t type, union TValue *value) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_string(const char *name, char *value_str, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

int32_t mhs_values_load_from_file(const char *filename, const char *section) ATTRIBUTE_INTERNAL;
int32_t mhs_value_set_string_list(const char *list, int32_t access, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

uint32_t mhs_value_get_status(const char *name, TMhsObjContext *context) ATTRIBUTE_INTERNAL;

#ifdef __cplusplus
  }
#endif

#endif
