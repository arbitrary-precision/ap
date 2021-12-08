#ifndef DEOHAYER_AP_AP_HPP
#define DEOHAYER_AP_AP_HPP

#include "integer.hpp"

template <unsigned long long _Bitwidth>
using ap_int = ap::library::integer<_Bitwidth, true>;

template <unsigned long long _Bitwidth>
using ap_uint = ap::library::integer<_Bitwidth, false>;

#endif
