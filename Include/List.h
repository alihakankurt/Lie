#ifndef __LIE_LIST_H__
#define __LIE_LIST_H__

#include <Core.h>
#include <Utility.h>

#define DeclareList(Name, Type)                                     \
    typedef struct Name                                             \
    {                                                               \
        usize Count;                                                \
        usize Capacity;                                             \
        Type* Values;                                               \
    } Name;                                                         \
                                                                    \
    void Initialize##Name(Name* list);                              \
    void Finalize##Name(Name* list);                                \
    void Clear##Name(Name* list);                                   \
    void AddTo##Name(Name* list, Type value);                       \
    bool InsertTo##Name(Name* list, Type value, usize insertIndex); \
    bool RemoveFrom##Name(Name* list, usize removeIndex);

#define ImplementList(Name, Type)                                            \
    void Initialize##Name(Name* list)                                        \
    {                                                                        \
        list->Count = 0;                                                     \
        list->Capacity = 0;                                                  \
        list->Values = NULL;                                                 \
    }                                                                        \
                                                                             \
    void Finalize##Name(Name* list)                                          \
    {                                                                        \
        MemoryFree(list->Values);                                            \
    }                                                                        \
                                                                             \
    void Extend##Name(Name* list)                                            \
    {                                                                        \
        usize capacity = (list->Capacity == 0) ? 2 : list->Capacity * 2;     \
        Type* values = (Type*)MemoryAllocate(capacity * sizeof(Type));       \
        if (list->Values != NULL)                                            \
        {                                                                    \
            MemoryCopy(values, list->Values, list->Count * sizeof(Type));    \
            MemoryFree(list->Values);                                        \
        }                                                                    \
                                                                             \
        list->Values = values;                                               \
        list->Capacity = capacity;                                           \
    }                                                                        \
                                                                             \
    void Clear##Name(Name* list)                                             \
    {                                                                        \
        list->Count = 0;                                                     \
    }                                                                        \
                                                                             \
    void AddTo##Name(Name* list, Type value)                                 \
    {                                                                        \
        if (list->Count == list->Capacity)                                   \
            Extend##Name(list);                                              \
                                                                             \
        list->Values[list->Count] = value;                                   \
        list->Count += 1;                                                    \
    }                                                                        \
                                                                             \
    bool InsertTo##Name(Name* list, Type value, usize insertIndex)           \
    {                                                                        \
        if (insertIndex > list->Count)                                       \
            return false;                                                    \
                                                                             \
        if (list->Count == list->Capacity)                                   \
            Extend##Name(list);                                              \
                                                                             \
        for (usize index = list->Count; index > insertIndex; index -= 1)     \
            list->Values[index] = list->Values[index - 1];                   \
                                                                             \
        list->Values[insertIndex] = value;                                   \
        list->Count += 1;                                                    \
        return true;                                                         \
    }                                                                        \
                                                                             \
    bool RemoveFrom##Name(Name* list, usize removeIndex)                     \
    {                                                                        \
        if (removeIndex >= list->Count)                                      \
            return false;                                                    \
                                                                             \
        for (usize index = removeIndex; index < list->Count - 1; index += 1) \
            list->Values[index] = list->Values[index + 1];                   \
                                                                             \
        list->Count -= 1;                                                    \
        return true;                                                         \
    }

#endif