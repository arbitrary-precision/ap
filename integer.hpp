#ifndef DEOHAYER_AP_INT_HPP
#define DEOHAYER_AP_INT_HPP

#include "integer_api.hpp"
#include "integer_handle.hpp"

namespace ap
{
namespace library
{
#define AP_DEFAULT_STR_BASE 10
#define AP_DEFAULT_STR "0123456789ABCDEF"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SFINAE

template <typename _IntegerL, typename _IntegerR, bool _Left>
struct wider_int;

template <typename _IntegerL, typename _IntegerR>
struct wider_int<_IntegerL, _IntegerR, true>
{
    using type = _IntegerL;
};

template <typename _IntegerL, typename _IntegerR>
struct wider_int<_IntegerL, _IntegerR, false>
{
    using type = _IntegerR;
};

template <typename _Type1, typename _Type2>
constexpr bool is_same_v()
{
    return std::is_same<_Type1, _Type2>::value;
}

template <typename _IntegerL, typename _IntegerR>
using wider_int_t = typename wider_int<_IntegerL, _IntegerR, (index_t(_IntegerL::valuewidth) >= index_t(_IntegerR::valuewidth)) >::type;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// integer class, represents user-level API

template <index_t _Maxbitwidth, bool _Signed>
class integer
{
public:
    template <index_t _MaxbitwidthO, bool _SignedO>
    friend class integer;

private:
    using handle_t = integer_handle<_Maxbitwidth>;
    handle_t handle;

public:
    enum : index_t
    {
        is_signed = _Signed,
        bitwidth = handle_t::bitwidth,
        wordwidth = handle_t::wordwidth,
        valuewidth = bitwidth - is_signed
    };

public:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ctor_d

    integer() = default;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // str

    integer& set(const char* str, index_t size = 0, index_t base = 0, const char* digits = AP_DEFAULT_STR)
    {
        fregister flags;
        size = ((size != 0) ? size : strlen(str));
        if (this->is_signed)
        {
            flags = sinteger_fstr(this->handle.get_wregister(), str, size, base, digits);
        }
        else
        {
            flags = uinteger_fstr(this->handle.get_wregister(), str, size, base, digits);
        }
        return *this;
    }

    integer& set(const std::string& str, index_t base = 0, const char* digits = AP_DEFAULT_STR)
    {
        return this->set(str.c_str(), str.size(), base, digits);
    }

    explicit integer(const std::string& str, index_t base = 0, const char* digits = AP_DEFAULT_STR)
        : integer(str.c_str(), index_t(str.size()), base, digits)
    {
    }

    explicit integer(const char* str, index_t size = 0, index_t base = 0, const char* digits = AP_DEFAULT_STR)
    {
        this->set(str, size, base, digits);
    }

    integer& operator=(const std::string& str)
    {
        return this->set(str);
    }

    integer& operator=(const char* str)
    {
        return this->set(str);
    }

    std::string str(index_t base = AP_DEFAULT_STR_BASE, const char* digits = AP_DEFAULT_STR) const
    {
        std::string str;
        rregister r = this->handle.get_rregister();
        if (this->is_signed)
        {
            sinteger_tstr(r, str, base, digits);
        }
        else
        {
            uinteger_tstr(r, str, base, digits);
        }
        return str;
    }

    explicit operator std::string() const
    {
        return this->str();
    }
};

#undef AP_DEFAULT_STR
#undef AP_DEFAULT_STR_BASE

} // namespace library
} // namespace ap

#endif
