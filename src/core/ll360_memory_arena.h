#pragma once
// Caller-owned linear allocator for short-lived port/runtime allocations.
// No internal heap allocation, locks, or platform APIs are used.
// The caller owns the memory passed to ArenaInit and must keep it alive.

#include <stddef.h>
#include <stdint.h>

namespace ll360
{
    struct MemoryArena
    {
        unsigned char* memory;
        size_t capacity;
        size_t used;
    };

    inline void ArenaInit(MemoryArena* arena, void* memory, size_t capacity)
    {
        if (!arena) return;
        arena->memory = static_cast<unsigned char*>(memory);
        arena->capacity = memory ? capacity : 0;
        arena->used = 0;
    }

    inline void ArenaReset(MemoryArena* arena)
    {
        if (arena) arena->used = 0;
    }

    inline size_t ArenaUsed(const MemoryArena* arena)
    {
        return arena ? arena->used : 0;
    }

    inline size_t ArenaRemaining(const MemoryArena* arena)
    {
        return (arena && arena->capacity >= arena->used) ? arena->capacity - arena->used : 0;
    }

    inline void* ArenaAlloc(MemoryArena* arena, size_t size, size_t alignment = 4)
    {
        if (!arena || !arena->memory || size == 0) return 0;
        if (alignment == 0) alignment = 1;
        // Alignment must be a power of two for this rounding formula.
        if ((alignment & (alignment - 1)) != 0) return 0;

        const uintptr_t base = reinterpret_cast<uintptr_t>(arena->memory);
        const uintptr_t current = base + arena->used;
        const uintptr_t aligned = (current + alignment - 1) & ~(static_cast<uintptr_t>(alignment) - 1);
        if (aligned < current) return 0; // integer overflow
        const size_t padding = static_cast<size_t>(aligned - current);
        if (padding > ArenaRemaining(arena)) return 0;
        if (size > ArenaRemaining(arena) - padding) return 0;

        arena->used += padding + size;
        return reinterpret_cast<void*>(aligned);
    }
}
