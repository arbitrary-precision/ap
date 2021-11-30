#ifndef DEOHAYER_AP_CONFIG_HPP
#define DEOHAYER_AP_CONFIG_HPP

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Userspace configuration.

// Core types.

#ifndef AP_WORD
#define AP_WORD unsigned
#endif

#ifndef AP_DWORD
#define AP_DWORD unsigned long long
#endif

#ifndef AP_SIZE
#define AP_SIZE unsigned
#endif

// Runtime assertion.

#ifndef AP_USE_ASSERT
#define AP_NO_ASSERT
#endif

// Linkage.

#ifndef AP_USE_SOURCES
#define AP_NO_USE_SOURCES
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal configuration.

// Core types.

#ifndef AP_WORD_SIZE
#define AP_WORD_SIZE sizeof(AP_WORD)
#endif

#ifndef AP_DWORD_SIZE
#define AP_DWORD_SIZE sizeof(AP_DWORD)
#endif

static_assert(AP_WORD_SIZE * 2 == AP_DWORD_SIZE);
static_assert(sizeof(AP_WORD) == AP_WORD_SIZE);
static_assert(sizeof(AP_DWORD) == AP_DWORD_SIZE);

// Debug.

#ifndef AP_NO_ASSERT
#include <exception>
#define AP_ASSERT(cond, ...)                                         \
    do                                                               \
    {                                                                \
        if (!(cond))                                                 \
        {                                                            \
            char buf[1024];                                          \
            sprintf(buf, __VA_ARGS__);                               \
            throw std::runtime_error{static_cast<const char*>(buf)}; \
        }                                                            \
    } while (0)
#else
#define AP_ASSERT(cond, ...)
#endif

// Linkage.

#ifndef AP_NO_USE_SOURCES
#define ap_linkage extern
#else
#define ap_linkage static inline
#endif

#endif
