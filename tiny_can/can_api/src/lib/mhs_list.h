#ifndef __MHS_LIST_H__
#define __MHS_LIST_H__

#ifdef __cplusplus
  extern "C" {
#endif


typedef struct _TMhsList TMhsList;

struct _TMhsList
  {
  TMhsList *Next;
  TMhsList *Prev;
  };


typedef struct _TMemAllocHeader TMemAllocHeader;

struct _TMemAllocHeader
  {
  TMhsList ListEntry;
  const char *File;
  uint32_t Line;
  };



#define mhs_is_list_empty(list) \
    (((TMhsList *)(list))->Next == ((TMhsList *)(list)))

#define mhs_remove_head_list(list) \
    ((TMhsList *)(list))->Next;\
    {mhs_remove_entry_list(((TMhsList *)(list))->Next)}

#define mhs_remove_entry_list(entry) {\
    TMhsList *ex_prev;\
    TMhsList *ex_next;\
    ex_next = ((TMhsList *)(entry))->Next;\
    ex_prev = ((TMhsList *)(entry))->Prev;\
    ex_prev->Next = ex_next;\
    ex_next->Prev = ex_prev;\
    }

#define mhs_insert_tail_list(list, entry) {\
    TMhsList *ex_prev;\
    TMhsList *ex_list;\
    ex_list = ((TMhsList *)(list));\
    ex_prev = ex_list->Prev;\
    ((TMhsList *)(entry))->Next = ex_list;\
    ((TMhsList *)(entry))->Prev = ex_prev;\
    ex_prev->Next = ((TMhsList *)(entry));\
    ex_list->Prev = ((TMhsList *)(entry));\
    }

#ifdef __cplusplus
  }
#endif

#endif
