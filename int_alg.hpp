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
// Unsigned algorithms declarations

ap_linkage std::string uint_tstr(const rregister& in, const index_t& base, const char* digits);

ap_linkage fregister uint_fstr(wregister& out, const std::string& str, const index_t& base, const char* digits);

ap_linkage cmpres uint_cmp(const rregister& left, const rregister& right);

ap_linkage void uint_ucp(const rregister& in, wregister& out);

ap_linkage fregister uint_scp(rregister in, wregister& out);

ap_linkage fregister uint_add(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_sub(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_mul(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_div(rregister left, rregister right, wregister& quo, wregister& rem);

ap_linkage fregister uint_or(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_xor(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_and(rregister left, rregister right, wregister& out);

ap_linkage fregister uint_not(rregister in, wregister& out);

ap_linkage fregister uint_rsh(rregister in, index_t shift, wregister& out);

ap_linkage fregister uint_lsh(rregister in, index_t shift, wregister& out);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Signed algorithms declarations

ap_linkage std::string sint_tstr(const rregister& in, const index_t& base, const char* digits);

ap_linkage fregister sint_fstr(wregister& out, const std::string& str, const index_t& base, const char* digits);

ap_linkage fregister sint_stou(rregister in, wregister& out);

ap_linkage fregister sint_utos(rregister in, wregister& out);

ap_linkage cmpres sint_cmp(const rregister& left, const rregister& right);

ap_linkage void sint_ucp(const rregister& in, wregister& out);

ap_linkage fregister sint_scp(rregister in, wregister& out);

ap_linkage fregister sint_add(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_sub(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_mul(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_div(rregister left, rregister right, wregister& quo, wregister& rem);

ap_linkage fregister sint_or(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_xor(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_and(rregister left, rregister right, wregister& out);

ap_linkage fregister sint_not(rregister in, wregister& out);

ap_linkage fregister sint_rsh(rregister in, index_t shift, wregister& out);

ap_linkage fregister sint_lsh(rregister in, index_t shift, wregister& out);

} // namespace library
} // namespace ap

#ifndef AP_USE_SOURCES
#define DEOHAYER_AP_INT_ALG_CPP 0
#include "int_alg.cpp"
#endif

#endif
