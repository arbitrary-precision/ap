#ifndef DEOHAYER_AP_INT_ALG_HPP
#define DEOHAYER_AP_INT_ALG_HPP

#include "asm.hpp"
#include <string>
#include <utility>

namespace ap
{
namespace library
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Size estimators

// Estimate number of word_t required to contain value represented by string of length str_size with given base.
static inline index_t est_fstr(index_t str_size, index_t base);

// Estimate size of string required to contain the value represented by in_size word_t values.
static inline index_t est_tstr(index_t in_size, index_t base);

static inline index_t est_lsh(index_t left_size, index_t bits);

static inline index_t est_rsh(index_t left_size, index_t bits);

static inline index_t est_add(index_t left_size, index_t right_size);

static inline index_t est_sub(index_t left_size, index_t right_size);

static inline index_t est_mul(index_t left_size, index_t right_size);

static inline index_t est_quo(index_t left_size, index_t right_size);

static inline index_t est_rem(index_t left_size, index_t right_size);

static inline index_t est_bit(index_t left_size, index_t right_size);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unsigned declarations
// Never change sign on unsigned values.

// Convert from string.
ap_linkage fregister uapi_fstr(wregister& out, const char* str, index_t size, index_t base, const char* digits);

// Convert to string.
ap_linkage void uapi_tstr(const rregister& in, std::string& str, index_t base, const char* digits, bool prefix = true);

// Convert from basic unsigned.
ap_linkage fregister uapi_fbasic(wregister& out, unsigned long long basic);

// Convert to basic unsigned.
ap_linkage unsigned long long uapi_tbasic(rregister in);

// Safe copy.
ap_linkage fregister uapi_scp(rregister in, wregister& out);

// Convert to signed.
ap_linkage fregister uapi_tos(rregister in, wregister& out);

// Comparison.
ap_linkage cmpres uapi_cmp(const rregister& left, const rregister& right);

// Addition.
ap_linkage fregister uapi_add(rregister left, rregister right, wregister& out);

// Subtraction.
ap_linkage fregister uapi_sub(rregister left, rregister right, wregister& out);

// Multiplication.
ap_linkage fregister uapi_mul(rregister left, rregister right, wregister& out);

// Division.
ap_linkage fregister uapi_div(rregister left, rregister right, wregister& quo, wregister& rem);

// Quotient.
ap_linkage fregister uapi_quo(rregister left, rregister right, wregister& out);

// Remainder.
ap_linkage fregister uapi_rem(rregister left, rregister right, wregister& out);

// Bitwise AND.
ap_linkage fregister uapi_and(rregister left, rregister right, wregister& out);

// Bitwise OR.
ap_linkage fregister uapi_or(rregister left, rregister right, wregister& out);

// Bitwise XOR.
ap_linkage fregister uapi_xor(rregister left, rregister right, wregister& out);

// Bitwise NOT.
ap_linkage fregister uapi_not(rregister in, wregister& out);

// Right bit shift.
ap_linkage fregister uapi_rsh(rregister in, index_t shift, wregister& out);

// Left bit shift.
ap_linkage fregister uapi_lsh(rregister in, index_t shift, wregister& out);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Signed declarations
// Properly set sign on signed values.

// Convert from string.
ap_linkage fregister sapi_fstr(wregister& out, const char* str, index_t size, index_t base, const char* digits);

// Convert to string.
ap_linkage void sapi_tstr(const rregister& in, std::string& str, index_t base, const char* digits, bool prefix = true);

// Convert from basic signed.
ap_linkage fregister sapi_fbasic(wregister& out, signed long long basic);

// Convert to basic signed.
ap_linkage signed long long sapi_tbasic(rregister in);

// Safe copy.
ap_linkage fregister sapi_scp(rregister in, wregister& out);

// Convert to unsigned.
ap_linkage fregister sapi_tou(rregister in, wregister& out);

// Addition.
ap_linkage fregister sapi_add(rregister left, rregister right, wregister& out);

// Subtraction.
ap_linkage fregister sapi_sub(rregister left, rregister right, wregister& out);

// Multiplication.
ap_linkage fregister sapi_mul(rregister left, rregister right, wregister& out);

// Division.
ap_linkage fregister sapi_div(rregister left, rregister right, wregister& quo, wregister& rem);

// Quotient.
ap_linkage fregister sapi_quo(rregister left, rregister right, wregister& out);

// Remainder.
ap_linkage fregister sapi_rem(rregister left, rregister right, wregister& out);

// Bitwise AND.
ap_linkage fregister sapi_and(rregister left, rregister right, wregister& out);

// Bitwise OR.
ap_linkage fregister sapi_or(rregister left, rregister right, wregister& out);

// Bitwise XOR.
ap_linkage fregister sapi_xor(rregister left, rregister right, wregister& out);

// Bitwise NOT.
ap_linkage fregister sapi_not(rregister in, wregister& out);

// Right bit shift.
ap_linkage fregister sapi_rsh(rregister in, index_t shift, wregister& out);

// Left bit shift.
ap_linkage fregister sapi_lsh(rregister in, index_t shift, wregister& out);

} // namespace library
} // namespace ap

#ifndef AP_USE_SOURCES
#define DEOHAYER_AP_INT_ALG_CPP 0
#include "int_api.cpp"
#endif

#endif
