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
#define AP_SIZE decltype(sizeof(0))
#endif

// Linkage.

#ifndef AP_USE_SOURCES
#define ap_linkage static inline
#else
#define ap_linkage extern
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

static_assert(AP_WORD_SIZE * 2 == AP_DWORD_SIZE, "AP: AP_DWORD_SIZE must be twice as large as AP_WORD_SIZE");
static_assert(sizeof(AP_WORD) == AP_WORD_SIZE, "AP: sizeof(AP_WORD) does not match AP_WORD_SIZE");
static_assert(sizeof(AP_DWORD) == AP_DWORD_SIZE, "AP: sizeof(AP_DWORD) does not match AP_DWORD_SIZE");

#endif
