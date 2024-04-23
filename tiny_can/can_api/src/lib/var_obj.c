/***************************************************************************
                          var_obj.c  -  description
                             -------------------
    begin             : 02.02.2012
    last modify       : 20.04.2023
    copyright         : (C) 2009 - 2023 by MHS-Elektronik GmbH & Co. KG, Germany
    author            : Klaus Demlehner, klaus@mhs-elektronik.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software, you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation.             *
 *                                                                         *
 ***************************************************************************/

/**
    Library to talk to Tiny-CAN devices. You find the latest versions at
       http://www.tiny-can.com/
**/
#include "global.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "ul_fifo.h"
#include "errors.h"
#include "mhs_class.h"
#include "mhs_obj.h"
#include "var_obj.h"

#define mhs_chk_context(c) do {if (!(c)) (c) = mhs_obj_context_default();} while(0)

/*
                        1 = Byte
                        2 = unsigned Byte
                        3 = Word
                        4 = unsigned Word
                        5 = Long
                        6 = unsigned Long
                        7 = Byte Array
                        8 = unsigned Byte Array
                        9 = Word Array
                       10 = unsigned Word Array
                       11 = Long Array
                       12 = unsigned Long Array
*/

static const uint16_t VAR_TYPE_LENGTH_TAB[] = {sizeof(int8_t),               // VT_BYTE
                                               sizeof(uint8_t),              // VT_UBYTE
                                               sizeof(int16_t),              // VT_WORD
                                               sizeof(uint16_t),             // VT_UWORD
                                               sizeof(int32_t),              // VT_LONG
                                               sizeof(uint32_t),             // VT_ULONG
                                               sizeof(int8_t),               // VT_BYTE_ARRAY
                                               sizeof(uint8_t),              // VT_UBYTE_ARRAY
                                               sizeof(int16_t),              // VT_WORD_ARRAY
                                               sizeof(uint16_t),             // VT_UWORD_ARRAY
                                               sizeof(int32_t),              // VT_LONG_ARRAY
                                               sizeof(uint32_t),             // VT_ULONG_ARRAY
                                               (sizeof(int8_t) * 2),         // VT_BYTE_RANGE_ARRAY
                                               (sizeof(uint8_t) * 2),        // VT_UBYTE_RANGE_ARRAY
                                               (sizeof(int16_t) * 2),        // VT_WORD_RANGE_ARRAY
                                               (sizeof(uint16_t) * 2),       // VT_UWORD_RANGE_ARRAY
                                               (sizeof(int32_t) * 2),        // VT_LONG_RANGE_ARRAY
                                               (sizeof(uint32_t) * 2)};      // VT_ULONG_RANGE_ARRAY



static void mhs_object_destroy_cb(TMhsObj *obj);


static const struct TObjFuncs ObjVarFuncs =
  {
  mhs_object_destroy_cb,
  NULL,
  NULL
  };


static void ulong_fifo_always_write(TULongFifo *fifo, uint32_t data)
{
if (fifo_full(fifo))
  ulong_fifo_clear(fifo);
ulong_fifo_write(fifo, data);
}


static int32_t mhs_get_value_size(TObjValue *value)
{
int32_t type;

if (!value)
  return(0);
type = value->Type;

if ((type >= VT_BYTE) && (type <= VT_ULONG))
  return(VAR_TYPE_LENGTH_TAB[type-1]);
else if (type == VT_HBYTE)
  return(sizeof(uint8_t));
else if (type == VT_HWORD)
  return(sizeof(uint16_t));
else if (type == VT_HLONG)
  return(sizeof(uint32_t));
else if (type == VT_STRING)
  {
  if (!value->Value.Str)
    return(0);
  else
    return((int32_t)strlen(value->Value.Str));
  }
return(0);
}


static void mhs_convert_ptr_to_value(union TValue *value, const void *ptr, int32_t type)
{
if (!value)
  return;
if (!ptr)
  {
  value->Ptr = NULL;
  return;
  }
switch(type)
  {
  case VT_BYTE              : {
                              value->S8 = *(int8_t *)ptr;
                              break;
                              }
  case VT_UBYTE             :
  case VT_HBYTE             : {
                              value->U8 = *(uint8_t *)ptr;
                              break;
                              }
  case VT_WORD              : {
                              value->S16 = *(int16_t *)ptr;
                              break;
                              }
  case VT_UWORD             :
  case VT_HWORD             : {
                              value->U16 = *(uint16_t *)ptr;
                              break;
                              }
  case VT_LONG              : {
                              value->S32 = *(int32_t *)ptr;
                              break;
                              }
  case VT_ULONG             :
  case VT_HLONG             : {
                              value->U32 = *(uint32_t *)ptr;
                              break;
                              }
  case VT_BYTE_ARRAY        :
  case VT_BYTE_RANGE_ARRAY  :
  case VT_UBYTE_ARRAY       :
  case VT_UBYTE_RANGE_ARRAY :
  case VT_WORD_ARRAY        :
  case VT_WORD_RANGE_ARRAY  :
  case VT_UWORD_ARRAY       :
  case VT_UWORD_RANGE_ARRAY :
  case VT_LONG_ARRAY        :
  case VT_LONG_RANGE_ARRAY  :
  case VT_ULONG_ARRAY       :
  case VT_ULONG_RANGE_ARRAY : {
                              // <*>
                              break;
                              }
  case VT_STREAM            : {
                              value->Ptr = (void *)ptr;
                              break;
                              }
  case VT_STRING            : {
                              value->Str = (char *)ptr;
                              break;
                              }
  }
}


static void mhs_value_write_to_mem(char *ptr, union TValue *value, int32_t type)
{
switch(type)
  {
  case VT_BYTE              :
  case VT_BYTE_ARRAY        :
  case VT_BYTE_RANGE_ARRAY  : {
                              *(int8_t *)ptr = value->S8;
                              break;
                              }
  case VT_UBYTE             :
  case VT_UBYTE_ARRAY       :
  case VT_UBYTE_RANGE_ARRAY :
  case VT_HBYTE             : {
                              *(uint8_t *)ptr = value->U8;
                              break;
                              }
  case VT_WORD              :
  case VT_WORD_ARRAY        :
  case VT_WORD_RANGE_ARRAY  : {
                              *(int16_t *)ptr = value->S16;
                              break;
                              }
  case VT_UWORD             :
  case VT_UWORD_ARRAY       :
  case VT_UWORD_RANGE_ARRAY :
  case VT_HWORD             : {
                              *(uint16_t *)ptr = value->U16;
                              break;
                              }
  case VT_LONG              :
  case VT_LONG_ARRAY        :
  case VT_LONG_RANGE_ARRAY  : {
                              *(int32_t *)ptr = value->S32;
                              break;
                              }
  case VT_ULONG             :
  case VT_ULONG_ARRAY       :
  case VT_ULONG_RANGE_ARRAY :
  case VT_HLONG             : {
                              *(uint32_t *)ptr = value->U32;
                              break;
                              }
  }
}


static void mhs_value_get_from_str(union TValue *value, char *value_str, int32_t type)
{
char *s;

if ((!value) || (!value_str))
  return;
switch(type)
  {
  case VT_BYTE              :
  case VT_BYTE_ARRAY        :
  case VT_BYTE_RANGE_ARRAY  : {
                              value->S8 = (int8_t)strtol(value_str, &s, 0);
                              break;
                              }
  case VT_UBYTE             :
  case VT_UBYTE_ARRAY       :
  case VT_UBYTE_RANGE_ARRAY :
  case VT_HBYTE             : {
                              value->U8 = (uint8_t)strtoul(value_str, &s, 0);
                              break;
                              }
  case VT_WORD              :
  case VT_WORD_ARRAY        :
  case VT_WORD_RANGE_ARRAY  : {
                              value->S16 = (int16_t)strtol(value_str, &s, 0);
                              break;
                              }
  case VT_UWORD             :
  case VT_UWORD_ARRAY       :
  case VT_UWORD_RANGE_ARRAY :
  case VT_HWORD             : {
                              value->U16 = (uint16_t)strtoul(value_str, &s, 0);
                              break;
                              }
  case VT_LONG              :
  case VT_LONG_ARRAY        :
  case VT_LONG_RANGE_ARRAY  : {
                              value->S32 = strtol(value_str, &s, 0);
                              break;
                              }
  case VT_ULONG             :
  case VT_ULONG_ARRAY       :
  case VT_ULONG_RANGE_ARRAY :
  case VT_HLONG             : {
                              value->U32 = strtoul(value_str, &s, 0);
                              break;
                              }
  }
}


static uint32_t mhs_uvalue_get(union TValue *value, int32_t type)
{
uint32_t value_out;

switch(type)
  {
  case VT_BYTE              :
  case VT_BYTE_ARRAY        :
  case VT_BYTE_RANGE_ARRAY  : {
                              value_out = (uint32_t)value->S8;
                              break;
                              }
  case VT_UBYTE             :
  case VT_UBYTE_ARRAY       :
  case VT_UBYTE_RANGE_ARRAY :
  case VT_HBYTE             : {
                              value_out = (uint32_t)value->U8;
                              break;
                              }
  case VT_WORD              :
  case VT_WORD_ARRAY        :
  case VT_WORD_RANGE_ARRAY  : {
                              value_out = (uint32_t)value->S16;
                              break;
                              }
  case VT_UWORD             :
  case VT_UWORD_ARRAY       :
  case VT_UWORD_RANGE_ARRAY :
  case VT_HWORD             : {
                              value_out = (uint32_t)value->U16;
                              break;
                              }
  case VT_LONG              :
  case VT_LONG_ARRAY        :
  case VT_LONG_RANGE_ARRAY  : {
                              value_out = (uint32_t)value->S32;
                              break;
                              }
  case VT_ULONG             :
  case VT_ULONG_ARRAY       :
  case VT_ULONG_RANGE_ARRAY :
  case VT_HLONG             : {
                              value_out = (uint32_t)value->U32;
                              break;
                              }
  default                   : value_out = 0;
  }
return(value_out);
}


static void mhs_object_destroy_cb(TMhsObj *obj)
{
TObjValue *value;

value = (TObjValue *)obj;
if (value->Type == VT_ULONG_FIFO)
  ulong_fifo_destroy((TULongFifo *)value->Value.Ptr);
else if ((value->Type >= VT_BYTE_ARRAY) && (value->Type <= VT_STRING))
  safe_free(value->Value.Ptr);
safe_free(value->Default);
}


/***************************************************************************/
/*                                                  */
/***************************************************************************/
TObjValue *mhs_value_create_unlocked(const char *name, int32_t type, int32_t max_size, TMhsObjContext *context)
{
TMhsObj *obj;
TObjValue *value;
int32_t mem_size;

if (mhs_object_get_by_name_unlocked(name, context))
  return(NULL);
#ifdef MHS_OBJ_USE_INDEX
obj = mhs_object_new_unlocked(OBJ_VALUE, 0, name, &ObjVarFuncs, sizeof(TObjValue), context);
#else
obj = mhs_object_new_unlocked(OBJ_VALUE, name, &ObjVarFuncs, sizeof(TObjValue), context);
#endif
if (!obj)
  return(NULL);
value = (TObjValue *)obj;
value->Type = type;
if (type == VT_ULONG_FIFO)
  {
  if (!(value->Value.Ptr = (void *)ulong_fifo_create(max_size)))
    {
    (void)mhs_object_remove(obj);
    return(NULL);
    }
  }
else if (((type >= VT_BYTE_ARRAY) && (type <= VT_ULONG_RANGE_ARRAY)) || (type == VT_STREAM))
  {
  value->MaxSize = max_size;
  if (type == VT_STREAM)
    mem_size = max_size;
  else
    mem_size = VAR_TYPE_LENGTH_TAB[type-1] * max_size;
  if (!(value->Value.Ptr = mhs_calloc(1, mem_size)))
    {
    (void)mhs_object_remove(obj);
    return(NULL);
    }
  }
return(value);
}


TObjValue *mhs_value_create(const char *name, int32_t type, int32_t max_size, TMhsObjContext *context)
{
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
obj = mhs_value_create_unlocked(name, type, max_size, context);
mhs_leave_critical(context->Event);
return(obj);
}


/***************************************************************************/
/*                                                  */
/***************************************************************************/
int32_t mhs_values_create_from_list(TMhsObjContext *context, const struct TValueDescription *list)
{
int32_t cnt, type;
const char *name;
TObjValue *obj;
union TValue value;
struct TValueDescription *l;

if (!list)
  return(0);
mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
cnt = 0;
for (l = (struct TValueDescription *)list; (name = l->Name); l++)
  {
  type = l->Type;
  if (!(obj = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
    {
    if (!(obj = mhs_value_create_unlocked(name, type, l->MaxSize, context)))
      break;
    }
  obj->Access = l->Access;
  obj->Flags = l->Flags;
  mhs_object_set_alias((TMhsObj *)obj, l->Alias);
  if (l->Default)
    {
    if (l->Flags & VF_DEF)
      {
      if (((type >= VT_BYTE) && (type <= VT_ULONG)) || ((type >= VT_HBYTE) && (type <= VT_HLONG)))
        obj->Default = mhs_memdup(l->Default, mhs_get_value_size(obj));
      else if (type == VT_STRING)
        obj->Default = mhs_strdup(l->Default);
      }
    mhs_convert_ptr_to_value(&value, l->Default, l->Type);
    if ((l->Type >= VT_BYTE_ARRAY) && (l->Type <= VT_ULONG_ARRAY))
      {
      //for (i = obj->MaxSize, i, i--)
      //  mhs_value_set_as_value_unlocked(obj, l->Type, &value);
      }
    else if (l->Type != VT_ULONG_FIFO)
      (void)mhs_value_set_as_value_unlocked(obj, l->Type, &value);
    }
  cnt++;
  }
mhs_leave_critical(context->Event);
return(cnt);
}


void mhs_value_set_default(TObjValue *obj)
{
int32_t type;
union TValue value;

if (!obj)
  return;
if (obj->Flags & VF_DEF)
  {
  type = obj->Type;
  if (((type >= VT_BYTE) && (type <= VT_ULONG)) ||
      ((type >= VT_HBYTE) && (type <= VT_HLONG)) ||
      (type == VT_STRING))
    {
    mhs_convert_ptr_to_value(&value, obj->Default, type);
    (void)mhs_value_set_as_value_unlocked(obj, type, &value);
    }
  }
}


void mhs_values_set_default(TMhsObjContext *context)
{
TMhsObj *obj;

if (!context)
  return;
mhs_enter_critical(context->Event);
for (obj = context->Items; obj; obj = obj->Next)
  {
  if (obj->Class == OBJ_VALUE)
    mhs_value_set_default((TObjValue *)obj);
  }
mhs_leave_critical(context->Event);
}


/*void mhs_values_set_all_default(void)
{
TMhsObjContext *list;

for (list = mhs_obj_get_context_list(); list; list = list->Next)
  {
  mhs_values_set_default(list);
  }
}*/


static void mhs_value_set_changed(TObjValue *value)
{
mhs_object_set_cmd_event((TMhsObj *)value);  // Commando Event
}


static int32_t mhs_value_set_ex(TObjValue **obj, const char *name, int32_t type, int32_t access, int32_t flags, TMhsObjContext *context)
{
TObjValue *o;
int32_t res;

res = 0;
// Object suchen/erzeugen
if (!(o = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
  {
  if (flags & VF_AUTO_CREATE)
    {
    if (!(o = mhs_value_create_unlocked(name, type, 0, context)))
      res = -1;  // <*> Fehler code
    }
  else
    res = -1;  // <*> Fehler code  
  }
if (obj)  
  *obj = o;
if (!o)
  return(res);
if (o->Type == VT_ULONG_FIFO)
  {
  if (type != VT_ULONG)
    return(ERR_VAR_WRONG_TYPE);
  }
else if (o->Type != type)
  return(ERR_VAR_WRONG_TYPE);
// Zugriffsrecht prüfen
#ifndef DISABLE_ACCESS_CHECK
if (!(access & o->Access))
  return(ERR_ACCESS_DENIED);
#endif
o->Status |= MHS_VAL_FLAG_WRITE;
return(0);
}


static int32_t mhs_value_set_data(const char *name, void *data, int32_t size, int32_t type, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t mem_size, err, change;
TObjValue *obj;

if (!context)
  return(0);
mhs_enter_critical(context->Event);
err = 0;
change = 0;
// Object suchen/erzeugen
if (!(obj = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
  {
  if (flags & VF_AUTO_CREATE)
    {
    if (!(obj = mhs_value_create_unlocked(name, type, 0, context)))
      err = -1;
    }
  }
if ((!err) && (!obj))
  err = -1;
if (!err)
  {
  // Zugriffsrecht prüfen
#ifndef DISABLE_ACCESS_CHECK
  if (!(access & obj->Access))
    err = ERR_ACCESS_DENIED;
  else
    {
#endif
    if (((type >= VT_BYTE_ARRAY) && (type <= VT_ULONG_RANGE_ARRAY)) || (type == VT_STREAM))
      {
      if (size > obj->MaxSize)
        err = -1;
      else
        {
        if (type == VT_STREAM)
          mem_size = size;
        else
          mem_size = VAR_TYPE_LENGTH_TAB[type-1] * size;
        if ((!obj->Value.Ptr) || (!data))
          err = -1;
        else
          {
          if ((obj->Size != size) || (memcmp(obj->Value.Ptr, data, mem_size)))
            {
            obj->Size = size;
            memcpy(obj->Value.Ptr, data, mem_size);
            change = 1;
            }
          }
        }
      }
#ifndef DISABLE_ACCESS_CHECK
    }
#endif
  }
if (!err)
  obj->Status |= MHS_VAL_FLAG_WRITE;
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}

// <*> Typen überprüfung fehlt
// <*> VT_ANY inplementieren
static int32_t mhs_value_get_ex(TObjValue **value, const char *name, int32_t type, int32_t access, TMhsObjContext *context)
{
TObjValue *v;
(void)type;

// Object suchen
if (!(v = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
  return(-1);
*value = v;
// Zugriffsrecht prüfen
#ifndef DISABLE_ACCESS_CHECK
if (!(access & v->Access))
  return(ERR_ACCESS_DENIED);
#endif
v->Status |= MHS_VAL_FLAG_READ;
return(0);
}


/***************************************************************************/
/*  "signed char" Value setzen, gegebenenfalls anlegen                     */
/***************************************************************************/
int32_t mhs_value_set_as_byte(const char *name, int8_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_BYTE, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.S8 != value)
    {
    obj->Value.S8 = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "uint8_t" Value setzen, gegebenenfalls anlegen                   */
/***************************************************************************/
int32_t mhs_value_set_as_ubyte(const char *name, uint8_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_UBYTE, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.U8 != value)
    {
    obj->Value.U8 = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "signed short" Value setzen, gegebenenfalls anlegen                    */
/***************************************************************************/
int32_t mhs_value_set_as_word(const char *name, int16_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_WORD, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.S16 != value)
    {
    obj->Value.S16 = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "uint16_t" Value setzen, gegebenenfalls anlegen                  */
/***************************************************************************/
int32_t mhs_value_set_as_uword(const char *name, uint16_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_UWORD, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.U16 != value)
    {
    obj->Value.U16 = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "int32_t" Value setzen, gegebenenfalls anlegen                     */
/***************************************************************************/
int32_t mhs_value_set_as_long(const char *name, int32_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_LONG, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.S32 != value)
    {
    obj->Value.S32 = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "uint32_t" Value setzen, gegebenenfalls anlegen                   */
/***************************************************************************/
int32_t mhs_value_set_as_ulong(const char *name, uint32_t value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_ULONG, access, flags, context)) >= 0)
  {
  if (obj->Type == VT_ULONG_FIFO)
    {
    ulong_fifo_always_write((TULongFifo *)obj->Value.Ptr, value);
    change = 1;
    }
  else
    {
    // Änderung ?
    if (obj->Value.U32 != value)
      {
      obj->Value.U32 = value;
      change = 1;
      }
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


int32_t mhs_value_set_as_ptr(const char *name, void *value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_POINTER, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (obj->Value.Ptr != value)
    {
    obj->Value.Ptr = value;
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "signed char" Array setzen, gegebenenfalls anlegen                     */
/***************************************************************************/
int32_t mhs_value_set_as_byte_array(const char *name, int8_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)array, size, VT_BYTE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint8_t" Array setzen, gegebenenfalls anlegen                         */
/***************************************************************************/
int32_t mhs_value_set_as_ubyte_array(const char *name, uint8_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)array, size, VT_UBYTE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "signed short" Array setzen, gegebenenfalls anlegen                    */
/***************************************************************************/
int32_t mhs_value_set_as_word_array(const char *name, int16_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)array, size, VT_WORD_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint16_t" Array setzen, gegebenenfalls anlegen                        */
/***************************************************************************/
int32_t mhs_value_set_as_uword_array(const char *name, uint16_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)array, size, VT_UWORD_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "int32_t" Array setzen, gegebenenfalls anlegen                         */
/***************************************************************************/
int32_t mhs_value_set_as_long_array(const char *name, int32_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)array, size, VT_LONG_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint32_t" Array setzen, gegebenenfalls anlegen                        */
/***************************************************************************/
int32_t mhs_value_set_as_ulong_array(const char *name, uint32_t *array, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void*)array, size, VT_ULONG_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "signed char" Array setzen, gegebenenfalls anlegen                     */
/***************************************************************************/
int32_t mhs_value_set_as_byte_rarray(const char *name, TByteRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)rarray, size, VT_BYTE_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint8_t" Array setzen, gegebenenfalls anlegen                         */
/***************************************************************************/
int32_t mhs_value_set_as_ubyte_rarray(const char *name, TUByteRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)rarray, size, VT_UBYTE_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "signed short" Array setzen, gegebenenfalls anlegen                    */
/***************************************************************************/
int32_t mhs_value_set_as_word_rarray(const char *name, TWordRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)rarray, size, VT_WORD_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint16_t" Array setzen, gegebenenfalls anlegen                        */
/***************************************************************************/
int32_t mhs_value_set_as_uword_rarray(const char *name, TUWordRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)rarray, size, VT_UWORD_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "int32_t" Array setzen, gegebenenfalls anlegen                         */
/***************************************************************************/
int32_t mhs_value_set_as_long_rarray(const char *name, TLongRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void *)rarray, size, VT_LONG_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "uint32_t" Array setzen, gegebenenfalls anlegen                        */
/***************************************************************************/
int32_t mhs_value_set_as_ulong_rarray(const char *name, TULongRange *rarray, int32_t size, int32_t access, int32_t flags, TMhsObjContext *context)
{
return(mhs_value_set_data(name, (void*)rarray, size, VT_ULONG_RANGE_ARRAY, access, flags, context));
}


/***************************************************************************/
/*  "string" Value setzen, gegebenenfalls anlegen                          */
/***************************************************************************/
int32_t mhs_value_set_as_string(const char *name, const char *value, int32_t access, int32_t flags, TMhsObjContext *context)
{
int32_t err, change;
TObjValue *obj;

mhs_chk_context(context);
if (!context)
  return(0);
change = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_set_ex(&obj, name, VT_STRING, access, flags, context)) >= 0)
  {
  // Änderung ?
  if (safe_strcmp(obj->Value.Str, value))
    {
    safe_free(obj->Value.Str);
    obj->Value.Str = mhs_strdup(value);
    change = 1;
    }
  }
mhs_leave_critical(context->Event);
if (((flags & VF_SET_CHANGE) || (change)) && (!(flags & VF_NO_EVENT)))
  mhs_value_set_changed(obj);
return(err);
}


/***************************************************************************/
/*  "signed char" Value lesen                                              */
/***************************************************************************/
int8_t mhs_value_get_as_byte(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int8_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_BYTE, access, context) < 0)
  value = 0;
else
  value = obj->Value.S8;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "uint8_t" Value lesen                                                  */
/***************************************************************************/
uint8_t mhs_value_get_as_ubyte(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
uint8_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UBYTE, access, context) < 0)
  value = 0;
else
  value = obj->Value.U8;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "signed short" Value lesen                                             */
/***************************************************************************/
int16_t mhs_value_get_as_word(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int16_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_WORD, access, context) < 0)
  value = 0;
else
  value = obj->Value.S16;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "uint16_t" Value lesen                                                 */
/***************************************************************************/
uint16_t mhs_value_get_as_uword(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
uint16_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UWORD, access, context) < 0)
  value = 0;
else
  value = obj->Value.U16;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "int32_t" Value lesen                                                  */
/***************************************************************************/
int32_t mhs_value_get_as_long(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_LONG, access, context) < 0)
  value = 0;
else
  value = obj->Value.S32;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "uint32_t" Value lesen                                                 */
/***************************************************************************/
uint32_t mhs_value_get_as_ulong(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
uint32_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ULONG, access, context) < 0)
  value = 0;
else
  value = obj->Value.U32;
mhs_leave_critical(context->Event);
return(value);
}


int32_t mhs_value_get_as_ulong_ex(const char *name, int32_t access, TMhsObjContext *context, uint32_t *value)
{
TObjValue *obj;
int32_t err;

mhs_chk_context(context);
if ((!context) || (!value))
  return(ERR_FATAL_INTERNEL_ERROR);
*value = 0;
mhs_enter_critical(context->Event);
if ((err = mhs_value_get_ex(&obj, name, VT_ULONG, access, context)) >= 0)
  {
  if (obj->Type == VT_ULONG_FIFO)
    err = ulong_fifo_read((TULongFifo *)obj->Value.Ptr, value);
  else
    *value = obj->Value.U32;
  }
mhs_leave_critical(context->Event);
return(err);
}


void *mhs_value_get_as_ptr(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
void *value;

mhs_chk_context(context);
if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_POINTER, access, context) < 0)
  value = NULL;
else
  value = obj->Value.Ptr;
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  Value als "uint32_t" lesen, automatisch konvertieren                   */
/***************************************************************************/
uint32_t mhs_value_get_as_unsigned(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
uint32_t value;

mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ANY, access, context) < 0)
  value = 0;
else
  value = mhs_uvalue_get(&obj->Value, obj->Type);
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  "signed char" Array lesen                                              */
/***************************************************************************/
int32_t mhs_value_get_as_byte_array(int8_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_BYTE_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int8_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "signed char" Array erstellen                               */
/***************************************************************************/
int32_t mhs_value_get_as_byte_array_dup(int8_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_BYTE_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int8_t *) mhs_memdup(obj->Value.Ptr, size);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint8_t" Array lesen                                                  */
/***************************************************************************/
int32_t mhs_value_get_as_ubyte_array(uint8_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UBYTE_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint8_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "uint8_t" Array erstellen                                   */
/***************************************************************************/
int32_t mhs_value_get_as_ubyte_array_dup(uint8_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UBYTE_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint8_t *) mhs_memdup(obj->Value.Ptr, size);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "signed short" Array lesen                                             */
/***************************************************************************/
int32_t mhs_value_get_as_word_array(int16_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_WORD_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int16_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "signed short" Array erstellen                              */
/***************************************************************************/
int32_t mhs_value_get_as_word_array_dup(int16_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_WORD_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int16_t *) mhs_memdup(obj->Value.Ptr, size * sizeof(int16_t));
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint16_t" Array lesen                                                 */
/***************************************************************************/
int32_t mhs_value_get_as_uword_array(uint16_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UWORD_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint16_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "uint16_t" Array erstellen                                  */
/***************************************************************************/
int32_t mhs_value_get_as_uword_array_dup(uint16_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UWORD_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint16_t *) mhs_memdup(obj->Value.Ptr, size * sizeof(uint16_t));
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "int32_t" Array lesen                                                  */
/***************************************************************************/
int32_t mhs_value_get_as_long_array(int32_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_LONG_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int32_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "int32_t" Array erstellen                                   */
/***************************************************************************/
int32_t mhs_value_get_as_long_array_dup(int32_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_LONG_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (int32_t *) mhs_memdup(obj->Value.Ptr, size * sizeof(int32_t));
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint32_t" Array lesen                                                 */
/***************************************************************************/
int32_t mhs_value_get_as_ulong_array(uint32_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

size = 0;
mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ULONG_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint32_t *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "uint32_t" Array erstellen                                  */
/***************************************************************************/
int32_t mhs_value_get_as_ulong_array_dup(uint32_t **array, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!array) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ULONG_ARRAY, access, context) < 0)
  *array = NULL;
else
  {
  size = obj->Size;
  *array = (uint32_t *) mhs_memdup(obj->Value.Ptr, size * sizeof(uint32_t));
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "signed char" Range-Array lesen                                        */
/***************************************************************************/
int32_t mhs_value_get_as_byte_rarray(TByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_BYTE_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TByteRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "signed char" Range-Array erstellen                         */
/***************************************************************************/
int32_t mhs_value_get_as_byte_rarray_dup(TByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_BYTE_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TByteRange *) mhs_memdup(obj->Value.Ptr, size * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint8_t" Range-Array lesen                                            */
/***************************************************************************/
int32_t mhs_value_get_as_ubyte_rarray(TUByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UBYTE_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TUByteRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "uint8_t" Range-Array erstellen                             */
/***************************************************************************/
int32_t mhs_value_get_as_ubyte_rarray_dup(TUByteRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UBYTE_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TUByteRange *) mhs_memdup(obj->Value.Ptr, size * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "signed short" Range-Array lesen                                       */
/***************************************************************************/
int32_t mhs_value_get_as_word_rarray(TWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_WORD_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TWordRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "signed short" Range-Array erstellen                        */
/***************************************************************************/
int32_t mhs_value_get_as_word_rarray_dup(TWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_WORD_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TWordRange *) mhs_memdup(obj->Value.Ptr, size * sizeof(int16_t) * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint16_t" Range-Array lesen                                           */
/***************************************************************************/
int32_t mhs_value_get_as_uword_rarray(TUWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UWORD_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TUWordRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "uint16_t" Range-Array erstellen                            */
/***************************************************************************/
int32_t mhs_value_get_as_uword_rarray_dup(TUWordRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_UWORD_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TUWordRange *) mhs_memdup(obj->Value.Ptr, size * sizeof(uint16_t) * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "int32_t" Range-Array lesen                                            */
/***************************************************************************/
int32_t mhs_value_get_as_long_rarray(TLongRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_LONG_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TLongRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  Eine Kopie "int32_t" Range-Array erstellen                             */
/***************************************************************************/
int32_t mhs_value_get_as_long_rarray_dup(TLongRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_LONG_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TLongRange *) mhs_memdup(obj->Value.Ptr, size * sizeof(int32_t) * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint32_t" Range-Array lesen                                           */
/***************************************************************************/
int32_t mhs_value_get_as_ulong_rarray(TULongRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ULONG_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TULongRange *)obj->Value.Ptr;
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "uint32_t" Range-Array lesen                                           */
/***************************************************************************/
int32_t mhs_value_get_as_ulong_rarray_dup(TULongRange **rarray, const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
int32_t size;

mhs_chk_context(context);
if ((!rarray) || (!context))
  return(-1);
size = 0;
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_ULONG_RANGE_ARRAY, access, context) < 0)
  *rarray = NULL;
else
  {
  size = obj->Size;
  *rarray = (TULongRange *) mhs_memdup(obj->Value.Ptr, size * sizeof(uint32_t) * 2);
  }
mhs_leave_critical(context->Event);
return(size);
}


/***************************************************************************/
/*  "string" Value lesen                                                   */
/***************************************************************************/
char *mhs_value_get_as_string(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
char *value, *s;

mhs_chk_context(context);
if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_STRING, access, context) < 0)
  value = NULL;
else
  {
  if ((s = obj->Value.Str))
    {                  // Prüfen ob der String nur Leerzeichen enthält
    while (*s == ' ')  // wenn ja -> NULL zurückgeben
      s++;
    if (*s == '\0')
      s = NULL;
    }
  if (s)
    value = obj->Value.Str;
  else
    value = NULL;
  }
mhs_leave_critical(context->Event);
return(value);
}


/***************************************************************************/
/*  Eine Kopie "string" Value erstellen                                    */
/***************************************************************************/
char *mhs_value_get_as_string_dup(const char *name, int32_t access, TMhsObjContext *context)
{
TObjValue *obj;
char *value, *s;

mhs_chk_context(context);
if (!context)
  return(NULL);
mhs_enter_critical(context->Event);
if (mhs_value_get_ex(&obj, name, VT_STRING, access, context) < 0)
  value = NULL;
else
  {
  if ((s = obj->Value.Str))
    {                  // Prüfen ob der String nur Leerzeichen enthält
    while (*s == ' ')  // wenn ja -> NULL zurückgeben
      s++;
    if (*s == '\0')
      s = NULL;
    }
  if (s)
    value = mhs_strdup(obj->Value.Str);
  else
    value = NULL;
  }
mhs_leave_critical(context->Event);
return(value);
}


int32_t mhs_value_set_as_value_unlocked(TObjValue *obj, int32_t type, union TValue *value)
{
int32_t change;

if (type != obj->Type)
  return(-1);      // Type konflikt
change = 0;
switch(type)
  {
  case VT_BYTE   : {
                   if (obj->Value.S8 != value->S8)
                     {
                     obj->Value.S8 = value->S8;
                     change = 1;
                     }
                   break;
                   }
  case VT_UBYTE  : {
                   if (obj->Value.U8 != value->U8)
                     {
                     obj->Value.U8 = value->U8;
                     change = 1;
                     }
                   break;
                   }
  case VT_WORD   : {
                   if (obj->Value.S16 != value->S16)
                     {
                     obj->Value.S16 = value->S16;
                     change = 1;
                     }
                   break;
                   }
  case VT_UWORD  : {
                   if (obj->Value.U16 != value->U16)
                     {
                     obj->Value.U16 = value->U16;
                     change = 1;
                     }
                   break;
                   }
  case VT_LONG   : {
                   if (obj->Value.S32 != value->S32)
                     {
                     obj->Value.S32 = value->S32;
                     change = 1;
                     }
                   break;
                   }
  case VT_ULONG  : {
                   if (obj->Value.U32 != value->U32)
                     {
                     obj->Value.U32 = value->U32;
                     change = 1;
                     }
                   break;
                   }
  case VT_STREAM : {
                   if (memcmp(obj->Value.Ptr, value->Ptr, obj->MaxSize))
                     {
                     safe_free(obj->Value.Ptr);
                     obj->Value.Ptr = mhs_malloc(obj->MaxSize);
                     memcpy(obj->Value.Ptr, value->Ptr, obj->MaxSize);
                     change = 1;
                     }
                   break;
                   }
  case VT_STRING : {
                   if (safe_strcmp(obj->Value.Str, value->Str))
                     {
                     safe_free(obj->Value.Str);
                     obj->Value.Str = mhs_strdup(value->Str);
                     change = 1;
                     }
                   break;
                   }
  }
return(change);
}


static int32_t mhs_value_set_range_array_from_string(TObjValue *obj, char *str)
{
char *s, *ptr;
int32_t type, res, await, count, element_size;
union TValue start, end;

await = 0;
start.U32 = 0L;
count = 0;
ptr = (char *)obj->Value.Ptr;
type = obj->Type;
element_size = VAR_TYPE_LENGTH_TAB[type-1] / 2;
obj->Size = 0;
memset(ptr, 0, element_size * 2 * obj->MaxSize);
while (*str)
  {
  // **** Bezeichner auslesen
  if (!(s = get_item_as_string(&str, "-,", GET_ITEM_DEF_FLAGS, &res)))
    return(-1);
  if (res < 0)
    return(-1);
  while (*s == ' ')
    s++;
  if (strlen(s) == 0)
    break;
  if (res == 1) // - gefunden
    {
    if (await != 0)
      return(-1);
    await = 1;
    mhs_value_get_from_str(&start, s, type);
    }
  else
    {
    if (await != 1)
      return(-1);
    await = 0;
    mhs_value_get_from_str(&end, s, type);

    count++;
    obj->Size = count;
    if (count > obj->MaxSize)
      return(ERR_VAR_SET_SIZE);           // Zu schreibende Variable zu groß
    mhs_value_write_to_mem(ptr, &start, type);
    ptr += element_size;
    mhs_value_write_to_mem(ptr, &end, type);
    ptr += element_size;
    }
  }
if (await)
  return(-1);
return(count);
}


int32_t mhs_value_set_string(const char *name, char *value_str, int32_t access, TMhsObjContext *context)
{
int32_t type, count, element_size, err;
TObjValue *obj;
union TValue value;
char *work_str, *s, *p, *item_str;

err = 0;
work_str = NULL;
mhs_chk_context(context);
if (!context)
  return(ERR_FATAL_INTERNEL_ERROR);
mhs_enter_critical(context->Event);
// Object suchen
if (!(obj = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
  err = ERR_VAR_NOT_FOUND;
// Zugriffsrecht prüfen
#ifndef DISABLE_ACCESS_CHECK
if (!err)
  {
  if (!(access & obj->Access))
    err = ERR_ACCESS_DENIED;
  }
#endif
if (!err)
  {
  obj->Status |= MHS_VAL_FLAG_WRITE;
  type = obj->Type;
  if ((type >= VT_BYTE_ARRAY) && (type <= VT_ULONG_ARRAY))
    {
    work_str = mhs_strdup(value_str);
    s = work_str;
    count = 0;
    element_size = VAR_TYPE_LENGTH_TAB[type-1];
    p = (char *)obj->Value.Ptr;
    do
      {
      item_str = get_item_as_string(&s, ",", GET_ITEM_DEF_FLAGS, &err);
      mhs_value_get_from_str(&value, item_str, type);
      mhs_value_write_to_mem(p, &value, type);
      if (err >= 0)
      	{
        count++;
        obj->Size = count;
        if (count > obj->MaxSize)
          {
          safe_free(work_str);
          err = ERR_VAR_SET_SIZE;           // Zu schreibende Variable zu groß
          break;
          }
        p += element_size;
        }
      }
    while (err >= 0);
    safe_free(work_str);
    }
  else if ((type >= VT_BYTE_RANGE_ARRAY) && (type <= VT_ULONG_RANGE_ARRAY))
    {
    work_str = mhs_strdup(value_str);
    mhs_value_set_range_array_from_string(obj, work_str);
    safe_free(work_str);
    }
  else
    {
    switch(type)
      {
      case VT_BYTE   : {
                       value.S8 = (int8_t)strtol(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_BYTE, &value);
                       break;
                       }
      case VT_UBYTE  : {
                       value.U8 = (uint8_t)strtoul(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_UBYTE, &value);
                       break;
                       }
      case VT_WORD   : {
                       value.S16 = (int16_t)strtol(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_WORD, &value);
                       break;
                       }
      case VT_UWORD  : {
                       value.U16 = (uint16_t)strtoul(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_UWORD, &value);
                       break;
                       }
      case VT_LONG   : {
                       value.S32 = strtol(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_LONG, &value);
                       break;
                       }
      case VT_ULONG  : {
                       value.U32 = strtoul(value_str, &s, 0);
                       err = mhs_value_set_as_value_unlocked(obj, VT_ULONG, &value);
                       break;
                       }
      case VT_STREAM : {
                       value.Str = value_str;
                       err = mhs_value_set_as_value_unlocked(obj, VT_STREAM, &value);
                       break;
                       }
      case VT_STRING : {
                       value.Str = value_str;
                       err = mhs_value_set_as_value_unlocked(obj, VT_STRING, &value);
                       break;
                       }
      default        : err = -1;
      }
    }
  }
mhs_leave_critical(context->Event);
if (err > 0)
  mhs_value_set_changed(obj);  // <*> bug, wird bei array Typen nicht aufgerufen
safe_free(work_str);
return(0);
}


/***************************************************************************/
/*  Eine Konfigurationsdatei laden                                         */
/***************************************************************************/
// <*> Von tcanprog_dll\lib übertragen !!!
/*int32_t mhs_values_load_from_file(const char *filename, const char *section)
{
TMhsObjContext *context;
FILE *file;
char line[MAX_LINE_SIZE];
char sec[MAX_LINE_SIZE];
char *str, *key, *value;

if (!filename)
  return(-1);
context = NULL;
if (!(file = fopen(filename, "r")))
  return(-1);

while (fgets(line, MAX_LINE_SIZE-1, file))
  {
  str = line;
  // Führende Leerzeichen löschen
  while (*str == ' ')
    str++;
  if (*str == ';' || *str == '#' || *str == 0)
    continue;
  if (sscanf(str, "[%[^]]", sec) == 1)
    {
    if (section)
      {
      if (context)
        break;
      if (safe_strcasecmp(sec, section))
        continue;
      if (!(context = mhs_obj_context_get_by_name(section)))
        break;
      }
    else
      context = mhs_obj_context_get_by_name(sec);
    continue;
    }
  if (context)
    {
    if (!(key = get_item_as_string(&str, "=", GET_ITEM_DEF_FLAGS, NULL)))
      continue;
    if (!(value = get_item_as_string(&str, ";#", GET_ITEM_DEF_FLAGS, NULL)))
      continue;
    (void)mhs_value_set_string(key, value, 0, context);
    }
  }
fclose(file);
return(0);
} */


int32_t mhs_value_set_string_list(const char *list, int32_t access, TMhsObjContext *context)
{
char *tmp_str, *str, *key, *value;
int32_t res;

if (!list)
  return(0);
tmp_str = mhs_strdup(list);
str = tmp_str;
while (*str)
  {
  // **** Key auslesen
  key = get_item_as_string(&str, "=;", GET_ITEM_DEF_FLAGS, &res);
  if ((res < 0) || (!key))
    break;
  // Führende Leerzeichen löschen
  while (*key == ' ')
    key++;
  if (*key == '\0')
    break;
  // **** Value auslesen
  value = get_item_as_string(&str, "=;", GET_ITEM_DEF_FLAGS, &res);
  if (res == 1) // = gefunden (Key) fehler
    break;
  // Führende Leerzeichen löschen
  if (value)
    {
    while (*value == ' ')
      value++;
    if (*value == '\0')
      value = NULL;
    }
  (void)mhs_value_set_string(key, value, access, context);
  }
safe_free(tmp_str);
return(0);
}


uint32_t mhs_value_get_status(const char *name, TMhsObjContext *context)
{
TObjValue *obj;
uint32_t status;

status = 0;
mhs_chk_context(context);
if (!context)
  return(0);
mhs_enter_critical(context->Event);
// Object suchen
if ((obj = (TObjValue *)mhs_object_get_by_name_unlocked(name, context)))
  status = obj->Status;
mhs_leave_critical(context->Event);
return(status);
}
