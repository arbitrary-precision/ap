#ifndef DEOHAYER_AP_INT_HPP
#define DEOHAYER_AP_INT_HPP

#include "integer_api.hpp"
#include "integer_handle.hpp"
#include <iostream>

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
using wider_int_t = typename wider_int<_IntegerL, _IntegerR, (index_t(_IntegerL::valuewidth) >= index_t(_IntegerR::valuewidth))>::type;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// integer class, represents user-level API

template <index_t _Bitwidth, bool _Signed>
class integer
{
    // Currently support for reverse narrowing is not available.
    static_assert(_Bitwidth > (sizeof(unsigned long long) * CHAR_BIT), "AP integer must be strictly greater than unsigned long long.");

public:
    // Divided we fall.
    template <index_t _BitwidthO, bool _SignedO>
    friend class integer;

private:
    // Holds actual data.
    using handle_t = integer_handle<_Bitwidth>;
    handle_t handle;

private:
    // Handle situations that occured during operations.

    static void fregister_handler_none(fregister flags)
    {
        AP_UNUSED(flags);
    }

    static void fregister_handler_div(fregister flags)
    {
        if (flags.has_any(fregister::infinity))
        {
            // Setting 0 / 0 explicitly may trigger compilation error.
            int a = 0;
            a = a / a;
        }
    }

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
    // ctor

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

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // basic

    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
    integer(T val)
    {
        wregister& reg = this->handle.get_wregister();
        if (this->is_signed)
        {
            if (std::is_signed<T>::value)
            {
                sinteger_fbasic(reg, val);
            }
            else
            {
                uinteger_fbasic(reg, val);
            }
        }
        else
        {
            if (std::is_signed<T>::value)
            {
                sinteger_fbasic(reg, val);
                sinteger_tou(this->handle.get_rregister(), reg);
            }
            else
            {
                uinteger_fbasic(reg, val);
            }
        }
    }

    explicit operator bool() const
    {
        return this->handle.get_size();
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
    explicit operator T() const
    {
        if (this->is_signed)
        {
            return static_cast<T>(sinteger_tbasic(this->handle.get_rregister()));
        }
        else
        {
            return static_cast<T>(uinteger_tbasic(this->handle.get_rregister()));
        }
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
    integer& operator=(T val)
    {
        *this = integer(val);
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // move

    integer(integer&& other) = default;

    integer& operator=(integer&& other) = default;

    template <index_t _BitwidthO, bool _SignedO, typename std::enable_if<!((_Bitwidth - _Signed) < (_BitwidthO - _SignedO)), bool>::type = false>
    integer(integer<_BitwidthO, _SignedO>&& other)
    {
        this->handle = std::move(other.handle);
        if (this->handle.get_sign() && !this->is_signed)
        {
            sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
        }
    }

    template <index_t _BitwidthO, bool _SignedO, typename std::enable_if<((_Bitwidth - _Signed) < (_BitwidthO - _SignedO)), bool>::type = false>
    explicit integer(integer<_BitwidthO, _SignedO>&& other)
    {
        index_t other_size = other.handle.get_size();
        this->handle = std::move(other.handle);
        if (this->is_signed)
        {
            if (((other_size > this->handle.get_capacity()) && this->handle.get_sign()))
            {
                sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
            }

            if (this->handle.get_rregister().has_msb())
            {
                uinteger_tos(this->handle.get_rregister(), this->handle.get_wregister());
            }
        }
        else
        {
            if (this->handle.get_sign() && !this->is_signed)
            {
                sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
            }
        }
    }

    template <index_t _BitwidthO, bool _SignedO>
    integer& operator=(integer<_BitwidthO, _SignedO>&& other)
    {
        *this = integer(std::move(other));
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // copy

    integer(const integer& other) = default;

    integer& operator=(const integer& other) = default;

    template <index_t _BitwidthO, bool _SignedO, typename std::enable_if<!((_Bitwidth - _Signed) < (_BitwidthO - _SignedO)), bool>::type = false>
    integer(const integer<_BitwidthO, _SignedO>& other)
    {
        this->handle = other.handle;
        if (this->handle.get_sign() && !this->is_signed)
        {
            sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
        }
    }

    template <index_t _BitwidthO, bool _SignedO, typename std::enable_if<((_Bitwidth - _Signed) < (_BitwidthO - _SignedO)), bool>::type = false>
    explicit integer(const integer<_BitwidthO, _SignedO>& other)
    {
        index_t other_size = other.handle.get_size();
        this->handle = other.handle;
        if (this->is_signed)
        {
            if (((other_size > this->handle.get_capacity()) && this->handle.get_sign()))
            {
                sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
            }

            if (this->handle.get_rregister().has_msb())
            {
                uinteger_tos(this->handle.get_rregister(), this->handle.get_wregister());
            }
        }
        else
        {
            if (this->handle.get_sign() && !this->is_signed)
            {
                sinteger_tou(this->handle.get_rregister(), this->handle.get_wregister());
            }
        }
    }

    template <index_t _BitwidthO, bool _SignedO>
    integer& operator=(const integer<_BitwidthO, _SignedO>& other)
    {
        *this = integer(other);
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // cmp

    template <typename T>
    bool operator<(const T& other) const
    {
        return this->cmp(other) == -1;
    }

    template <typename T>
    bool operator<=(const T& other) const
    {
        return this->cmp(other) != 1;
    }

    template <typename T>
    bool operator>(const T& other) const
    {
        return this->cmp(other) == 1;
    }

    template <typename T>
    bool operator>=(const T& other) const
    {
        return this->cmp(other) != -1;
    }

    template <typename T>
    bool operator==(const T& other) const
    {
        return this->cmp(other) == 0;
    }

    template <typename T>
    bool operator!=(const T& other) const
    {
        return this->cmp(other) != 0;
    }

    int cmp(const integer& other) const
    {
        if (this->is_signed)
        {
            return sinteger_cmp(this->handle.get_rregister(), other.handle.get_rregister()).result;
        }
        else
        {
            return uinteger_cmp(this->handle.get_rregister(), other.handle.get_rregister()).result;
        }
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
    int cmp(const T& other) const
    {
        if (other == 0)
        {
            if (this->handle.get_size() != 0)
            {
                return this->handle.get_size() > 0 ? 1 : -1;
            }
            else
            {
                return 0;
            }
        }
        return this->cmp(integer(other));
    }

    template <typename T, typename std::enable_if<bool(T::valuewidth), bool>::type = false>
    int cmp(const T& other) const
    {
        if (_Signed != bool(other.is_signed))
        {
            if (_Bitwidth > (other.valuewidth))
            {
                return this->cmp(integer(other));
            }
            else
            {
                return T(*this).cmp(other);
            }
        }
        else
        {
            if (this->is_signed)
            {
                return sinteger_cmp(this->handle.get_rregister(), other.handle.get_rregister()).result;
            }
            else
            {
                return uinteger_cmp(this->handle.get_rregister(), other.handle.get_rregister()).result;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ordinary binary operators

#define AP_BINARY_OPERATOR(name, op, fhandler)                                                      \
    template <typename T, typename std::enable_if<bool(T::valuewidth), bool>::type = false>         \
    wider_int_t<integer<_Bitwidth, _Signed>, T> op(const T& other) const                            \
    {                                                                                               \
        wider_int_t<integer<_Bitwidth, _Signed>, T> result;                                         \
        this->dispatch_binary_operation<uinteger_##name, sinteger_##name, fhandler>(other, result); \
        return result;                                                                              \
    }                                                                                               \
                                                                                                    \
    template <typename T, typename std::enable_if<bool(T::valuewidth), bool>::type = false>         \
    integer& op## = (const T& other)                                                                \
    {                                                                                               \
        this->dispatch_binary_operation<uinteger_##name, sinteger_##name, fhandler>(other, *this);  \
        return *this;                                                                               \
    }                                                                                               \
                                                                                                    \
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>  \
    integer op(const T& other) const                                                                \
    {                                                                                               \
        return this->op(integer(other));                                                            \
    }                                                                                               \
                                                                                                    \
    template <typename T, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>  \
    integer& op## = (const T& other)                                                                \
    {                                                                                               \
        return this->op## = (integer(other));                                                       \
    }

    AP_BINARY_OPERATOR(add, operator+, fregister_handler_none)
    AP_BINARY_OPERATOR(sub, operator-, fregister_handler_none)
    AP_BINARY_OPERATOR(mul, operator*, fregister_handler_none)
    AP_BINARY_OPERATOR(quo, operator/, fregister_handler_div)
    AP_BINARY_OPERATOR(rem, operator%, fregister_handler_div)
    AP_BINARY_OPERATOR(and, operator&, fregister_handler_none)
    AP_BINARY_OPERATOR(or, operator|, fregister_handler_none)
    AP_BINARY_OPERATOR(xor, operator^, fregister_handler_none)

    integer operator~() const
    {
        integer result;
        if (_Signed)
        {
            sinteger_not(this->handle.get_rregister(), result.handle.get_wregister());
        }
        else
        {
            uinteger_not(this->handle.get_rregister(), result.handle.get_wregister());
        }
        return result;
    }

    integer operator++(int)
    {
        integer result = *this;
        *this += integer(1);
        return result;
    }

    integer& operator++()
    {
        return *this += integer(1);
    }

    integer operator--(int)
    {
        integer result = *this;
        *this -= integer(1);
        return result;
    }

    integer& operator--()
    {
        *this -= integer(1);
        return *this;
    }

    integer operator-() const
    {
        integer result = *this;
        if (this->handle.get_size() != 0)
        {
            wregister& reg = result.handle.get_wregister();
            if (_Signed)
            {
                if (!reg.has_msb())
                {
                    reg.sign = !reg.sign;
                }
            }
            else
            {
                reg.sign = 1;
                sinteger_tou(rregister(reg), reg);
            }
        }
        return result;
    }

    integer operator+() const
    {
        return *this;
    }

    integer operator>>(unsigned long long int shift) const
    {
        integer result;
        if (_Signed)
        {
            sinteger_rsh(this->handle.get_rregister(), shift, result.handle.get_wregister());
        }
        else
        {
            uinteger_rsh(this->handle.get_rregister(), shift, result.handle.get_wregister());
        }
        return result;
    }

    integer& operator>>=(unsigned long long int shift)
    {
        if (_Signed)
        {
            sinteger_rsh(this->handle.get_rregister(), shift, this->handle.get_wregister());
        }
        else
        {
            uinteger_rsh(this->handle.get_rregister(), shift, this->handle.get_wregister());
        }
        return *this;
    }

    integer operator<<(unsigned long long int shift) const
    {
        integer result;
        if (_Signed)
        {
            sinteger_lsh(this->handle.get_rregister(), shift, result.handle.get_wregister());
        }
        else
        {
            uinteger_lsh(this->handle.get_rregister(), shift, result.handle.get_wregister());
        }
        return result;
    }

    integer& operator<<=(unsigned long long int shift)
    {
        (*this) = (*this) << shift;
        return *this;
    }

private:
    using binary_operation = fregister (*)(rregister, rregister, wregister&);
    using fregister_handler = void (*)(fregister);

    template <binary_operation uop, binary_operation sop, fregister_handler h,
              index_t _BitwidthO, bool _SignedO,
              index_t _BitwidthR, bool _SignedR>
    void dispatch_binary_operation(const integer<_BitwidthO, _SignedO>& other, integer<_BitwidthR, _SignedR>& result) const
    {
        rregister left;
        rregister right;
        fregister flags;

        if (_SignedO == _Signed)
        {
            left = this->handle.get_rregister();
            right = other.handle.get_rregister();
            wregister& out = result.handle.get_wregister();
            binary_operation op = (result.is_signed ? sop : uop);
            out.sign = 0;
            flags = op(left, right, out);
        }
        else
        {
            wregister& out = result.handle.get_wregister();
            if ((_BitwidthO - _SignedO) > (_Bitwidth - _Signed))
            {
                integer<_BitwidthO, _SignedO> nleft{*this};
                left = nleft.handle.get_rregister();
                right = other.handle.get_rregister();
                binary_operation op = (_SignedO ? sop : uop);
                out.sign = 0;
                flags = op(left, right, out);
                if (result.is_signed != _SignedO)
                {
                    if (_SignedO)
                    {
                        sinteger_tou(rregister(out), out);
                    }
                    else
                    {
                        uinteger_tos(rregister(out), out);
                    }
                }
            }
            else
            {
                integer<_Bitwidth, _Signed> nright{other};
                left = this->handle.get_rregister();
                right = nright.handle.get_rregister();
                binary_operation op = (_Signed ? sop : uop);
                out.sign = 0;
                flags = op(left, right, out);
                if (result.is_signed != _Signed)
                {
                    if (_Signed)
                    {
                        sinteger_tou(rregister(out), out);
                    }
                    else
                    {
                        uinteger_tos(rregister(out), out);
                    }
                }
            }
        }

        h(flags);
    }
};

} // namespace library
} // namespace ap

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator<(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) != -1;
}

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator<=(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) != -1;
}

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator>(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) != 1;
}

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator>=(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) != 1;
}

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator==(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) == 0;
}

template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false>
bool operator!=(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)
{
    return right.cmp(left) != 0;
}

#define AP_BASIC_OPERATOR(op)                                                                                                                    \
    template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false> \
    ap::library::integer<_Bitwidth, _Signed> operator op(const T& left, const ap::library::integer<_Bitwidth, _Signed>& right)                   \
    {                                                                                                                                            \
        return ap::library::integer<_Bitwidth, _Signed>(left) op right;                                                                          \
    }                                                                                                                                            \
                                                                                                                                                 \
    template <typename T, ap::library::index_t _Bitwidth, bool _Signed, typename std::enable_if<std::is_integral<T>::value, bool>::type = false> \
    T& operator op##=(T& left, const ap::library::integer<_Bitwidth, _Signed>& right)                                                            \
    {                                                                                                                                            \
        left = static_cast<T>(ap::library::integer<_Bitwidth, _Signed>(left) op right);                                                          \
        return left;                                                                                                                             \
    }

AP_BASIC_OPERATOR(+)
AP_BASIC_OPERATOR(-)
AP_BASIC_OPERATOR(*)
AP_BASIC_OPERATOR(/)
AP_BASIC_OPERATOR(%)
AP_BASIC_OPERATOR(&)
AP_BASIC_OPERATOR(^)
AP_BASIC_OPERATOR(|)
AP_BASIC_OPERATOR(<<)
AP_BASIC_OPERATOR(>>)

template <ap::library::index_t _Bitwidth, bool _Signed>
std::ostream& operator<<(std::ostream& os, const ap::library::integer<_Bitwidth, _Signed>& val)
{
    ap::library::index_t base = 10;
    if (os.flags() & std::ios_base::oct)
    {
        base = 8;
    }
    if (os.flags() & std::ios_base::hex)
    {
        base = 16;
    }
    os << val.str(base);
    return os;
}

template <ap::library::index_t _Bitwidth, bool _Signed>
std::istream& operator>>(std::istream& is, ap::library::integer<_Bitwidth, _Signed>& val)
{
    std::string str;
    is >> str;
    val = str;
    return is;
}

#undef AP_BASIC_OPERATOR
#undef AP_BINARY_OPERATOR
#undef AP_DEFAULT_STR
#undef AP_DEFAULT_STR_BASE

#endif
