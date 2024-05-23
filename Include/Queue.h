#ifndef __LIE_QUEUE_H__
#define __LIE_QUEUE_H__

#include <Core.h>
#include <Utility.h>

#define DeclareQueue(Name, Type)                  \
    typedef struct Name                           \
    {                                             \
        usize Back;                               \
        usize Front;                              \
        usize Count;                              \
        usize Capacity;                           \
        Type* Values;                             \
    } Name;                                       \
                                                  \
    void Initialize##Name(Name* queue);           \
    void Finalize##Name(Name* queue);             \
    void Enqueue##Name(Name* queue, Type value);  \
    bool Dequeue##Name(Name* queue, Type* value); \
    void Clear##Name(Name* queue);

#define ImplementQueue(Name, Type)                                                                                                           \
    void Initialize##Name(Name* queue)                                                                                                       \
    {                                                                                                                                        \
        queue->Back = 0;                                                                                                                     \
        queue->Front = 0;                                                                                                                    \
        queue->Count = 0;                                                                                                                    \
        queue->Capacity = 4;                                                                                                                 \
        queue->Values = (Type*)MemoryAllocate(queue->Capacity * sizeof(Type));                                                               \
    }                                                                                                                                        \
                                                                                                                                             \
    void Finalize##Name(Name* queue)                                                                                                         \
    {                                                                                                                                        \
        MemoryFree(queue->Values);                                                                                                           \
        queue->Values = NULL;                                                                                                                \
    }                                                                                                                                        \
                                                                                                                                             \
    void Enqueue##Name(Name* queue, Type value)                                                                                              \
    {                                                                                                                                        \
        if (queue->Back == queue->Front && queue->Count > 0)                                                                                 \
        {                                                                                                                                    \
            usize capacity = queue->Capacity * 2;                                                                                            \
            Type* values = (Type*)MemoryAllocate(capacity * sizeof(Type));                                                                   \
            MemoryCopy(&values[0], &queue->Values[0], queue->Front * sizeof(Type));                                                          \
            MemoryCopy(&values[queue->Back + queue->Capacity], &queue->Values[queue->Back], (queue->Capacity - queue->Back) * sizeof(Type)); \
            MemoryFree(queue->Values);                                                                                                       \
            queue->Back += queue->Capacity;                                                                                                  \
            queue->Capacity = capacity;                                                                                                      \
            queue->Values = values;                                                                                                          \
        }                                                                                                                                    \
                                                                                                                                             \
        queue->Values[queue->Front] = value;                                                                                                 \
        queue->Front += 1;                                                                                                                   \
        queue->Count += 1;                                                                                                                   \
                                                                                                                                             \
        if (queue->Front == queue->Capacity)                                                                                                 \
        {                                                                                                                                    \
            queue->Front = 0;                                                                                                                \
        }                                                                                                                                    \
    }                                                                                                                                        \
                                                                                                                                             \
    bool Dequeue##Name(Name* queue, Type* value)                                                                                             \
    {                                                                                                                                        \
        if (queue->Count == 0)                                                                                                               \
        {                                                                                                                                    \
            return false;                                                                                                                    \
        }                                                                                                                                    \
                                                                                                                                             \
        *value = queue->Values[queue->Back];                                                                                                 \
        queue->Back += 1;                                                                                                                    \
        queue->Count -= 1;                                                                                                                   \
                                                                                                                                             \
        if (queue->Back == queue->Capacity)                                                                                                  \
        {                                                                                                                                    \
            queue->Back = 0;                                                                                                                 \
        }                                                                                                                                    \
                                                                                                                                             \
        return true;                                                                                                                         \
    }                                                                                                                                        \
                                                                                                                                             \
    void Clear##Name(Name* queue)                                                                                                            \
    {                                                                                                                                        \
        queue->Back = 0;                                                                                                                     \
        queue->Front = 0;                                                                                                                    \
        queue->Count = 0;                                                                                                                    \
    }

#endif