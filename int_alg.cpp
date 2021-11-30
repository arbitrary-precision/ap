#ifndef DEOHAYER_AP_INT_ALG_CPP
#define DEOHAYER_AP_INT_ALG_CPP 1
#endif
#if (DEOHAYER_AP_INT_ALG_CPP == 1) == defined(AP_USE_SOURCES)

#include "asm.hpp"
#include <utility>

namespace ap
{
namespace library
{

#define WRAP(rreg, wreg)                    \
    do                                      \
    {                                       \
        if (wreg.capacity < rreg.size)      \
        {                                   \
            rreg.size = wreg.capacity;      \
            flags.set(fregister::wrapping); \
        }                                   \
    } while (false)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unsigned algorithms definitions

std::string uint_tstr(const rregister& in, const index_t& base, const char* digits)
{
    std::string result;
    return result;
}

fregister uint_fstr(wregister& out, const std::string& str, const index_t& base, const char* digits)
{
    fregister flags;
    return flags;
}

cmpres uint_cmp(const rregister& left, const rregister& right)
{
    cmpres result;
    return result;
}

void uint_ucp(const rregister& in, wregister& out)
{
}

fregister uint_scp(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_add(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_sub(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_mul(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_div(rregister left, rregister right, wregister& quo, wregister& rem)
{
    fregister flags;
    return flags;
}

fregister uint_or(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_xor(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_and(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_not(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_rsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    return flags;
}

fregister uint_lsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    return flags;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Signed algorithms definitions

std::string sint_tstr(const rregister& in, const index_t& base, const char* digits)
{
    std::string result;
    return result;
}

fregister sint_fstr(wregister& out, const std::string& str, const index_t& base, const char* digits)
{
    fregister flags;
    return flags;
}

fregister sint_stou(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_utos(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

cmpres sint_cmp(const rregister& left, const rregister& right)
{
    cmpres result;
    return result;
}

void sint_ucp(const rregister& in, wregister& out)
{
}

fregister sint_scp(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_add(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_sub(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_mul(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_div(rregister left, rregister right, wregister& quo, wregister& rem)
{
    fregister flags;
    return flags;
}

fregister sint_or(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_xor(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_and(rregister left, rregister right, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_not(rregister in, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_rsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    return flags;
}

fregister sint_lsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    return flags;
}

} // namespace library
} // namespace ap

#endif