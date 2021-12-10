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

// Utilities

// Helper function. In this context normalization means the following:
//  1. Handle overflow - if original value was negative, then two's complement pattern must be restored.
//  2. Handle msb - regardless whether overflow occured, set pattern and sign according to msb.
//  3. Remove any leading zeroes and remove sign on zero value.
// This is specific to signed representation only. For unsigned normalization is just asm_trim().
static inline fregister snorm(wregister& out, fregister flags)
{
    if (out.has_msb())
    {
        asm_twos(rregister(out), out);
        if (out.has_msb())
        {
            out.sign = 1;
        }
        else
        {
            out.sign += 1;
            flags.set(fregister::overflow);
        }
    }
    asm_trim(out);
    if (out.size == 0)
    {
        out.sign = 0;
    }
    return flags;
}

// Calculate how many bits are needed for base (string conversion utility, base is up to 256).
static inline index_t min_bits_for_base(index_t base)
{
    static const index_t powers[8] = {1, 2, 4, 8, 18, 32, 64, 128};
    base = ((base == 0) ? 16 : base);
    index_t i = 0;
    for (i = 0; i < sizeof(powers) / sizeof(powers[0]); ++i)
    {
        if (base <= powers[i])
        {
            break;
        }
    }
    return i;
}

// Get prefix for the given 2, 8 or 16 base.
static inline std::string prefix_for_base(index_t base)
{
    if (base == 2)
    {
        return "0b";
    }
    else if (base == 8)
    {
        return "0";
    }
    else if (base == 16)
    {
        return "0x";
    }
    return "";
}

// Adjust reg1 so that its size does not exceed reg2 capacity. Set flags accordingly.
#define WRAP(reg1, reg2)                \
    if (reg1.size > reg2.capacity)      \
    {                                   \
        reg1.size = reg2.capacity;      \
        reg1.capacity = reg2.capacity;  \
        flags.set(fregister::overflow); \
    }

// asm bit operation
using asm_bit_op = void (*)(const rregister&, const rregister&, wregister&);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Size estimators

index_t est_fstr(index_t str_size, index_t base)
{
    return ((str_size * min_bits_for_base(base)) / word_traits::bits) + 1;
}

index_t est_tstr(index_t in_size, index_t base)
{
    if (base == 2)
    {
        return in_size * word_traits::bits;
    }
    else
    {
        return ((in_size * word_traits::bits) / min_bits_for_base(base)) * 2;
    }
}

index_t est_add(index_t left_size, index_t right_size)
{
    return AP_MAX(left_size, right_size) + 1;
}

index_t est_sub(index_t left_size, index_t right_size)
{
    return est_add(left_size, right_size);
}

index_t est_mul(index_t left_size, index_t right_size)
{
    return left_size + right_size;
}

index_t est_quo(index_t left_size, index_t right_size)
{
    if (left_size < right_size)
    {
        return 1;
    }
    return left_size - right_size + 1;
}

index_t est_rem(index_t left_size, index_t right_size)
{
    (void)left_size;
    return right_size;
}

index_t est_bit(index_t left_size, index_t right_size)
{
    return AP_MAX(left_size, right_size);
}

index_t est_lsh(index_t left_size, index_t bits)
{
    index_t words = bits / word_traits::bits + bool(bits % word_traits::bits);
    return left_size + words;
}

index_t est_rsh(index_t left_size, index_t bits)
{
    index_t words = bits / word_traits::bits + bool(bits % word_traits::bits);
    if (left_size < words)
    {
        return 0;
    }
    return left_size - words;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// uapi definitions.

fregister uinteger_fstr(wregister& out, const char* str, index_t size, index_t base, const char* digits)
{
    fregister flags{};
    // Arithmetic base for out.
    static constexpr dword_t divisor = dword_t{word_traits::ones} + 1;
    out.size = 0;
    // Trivial case.
    if (size == 0)
    {
        return flags;
    }
    // Register to hold the full value. Memory will be allocated later.
    wregister nout{nullptr, 0, 0, str[0] == '-'};
    // Adjust if sign is present.
    if (nout.sign == 1 || str[0] == '+')
    {
        ++str;
        --size;
    }
    // Detectable bases are 2 ("0b", "0B"), 8 ("0"), 10 (default), 16 ("0x", "0X").
    index_t detected_base = 10;
    if (str[0] == '0')
    {
        if (size > 1)
        {
            ++str;
            --size;
            if (str[0] == 'x' || str[0] == 'X')
            {
                detected_base = 16;
                ++str;
                --size;
            }
            else if (str[0] == 'b' || str[0] == 'B')
            {
                detected_base = 2;
                ++str;
                --size;
            }
            else
            {
                detected_base = 8;
            }
        }
    }
    if (base == 0)
    {
        base = detected_base;
    }
    // Transform str to raw form, where each symbol is mapped to [0, base) value.
    // raw_str will be a backwards representation of str, with raw_str[0] as the most significant digit.
    auto raw_str = array_alloc<unsigned char>(size);
    index_t raw_size = size;
    for (index_t i = 0; i < raw_size; ++i)
    {
        index_t j = 0;
        // Find symbol in digits array, index will be its value.
        for (j = 0; j < base; ++j)
        {
            // If symbol found in digits, append its value to raw_str.
            if (str[i] == digits[j])
            {
                raw_str[i] = static_cast<unsigned char>(j);

                break;
            }
        }
        // Unknown symbol encountered.
        if (j == base)
        {
            raw_size = i;
            break;
        }
    }
    // Allocate memory for nout to hold the full value.
    nout.capacity = est_fstr(raw_size, base);
    auto nout_words = array_alloc<word_t>(nout.capacity);
    nout.words = nout_words.get();
    // Perform base translation using classical approach.
    const dword_t raw_base = base;
    dword_t dividend = 0;
    index_t str_i = 0; // for reading values from raw_str
    index_t str_j = 0; // for writing values to raw_str
    while (raw_size > 0)
    {
        // Perform single division.
        str_i = 0;
        str_j = 0;
        dividend = 0;
        // Get the first digit of the result.
        for (str_i = 0; str_i < raw_size; ++str_i)
        {
            dividend *= raw_base;
            dividend += raw_str[str_i];
            if (dividend >= divisor)
            {
                raw_str[str_j] = static_cast<unsigned char>(dividend / divisor);
                ++str_j;
                dividend %= divisor;
                break;
            }
        }
        // Case of the last division.
        if (str_j == 0)
        {
            nout.words[nout.size] = static_cast<word_t>(dividend);
            ++nout.size;
            break;
        }
        // At this point, for loop above was terminated through break.
        // str_i still refers to already processed element.
        ++str_i;
        // Get the rest of the digits.
        while (str_i < raw_size)
        {

            dividend *= raw_base;
            dividend += raw_str[str_i];
            ++str_i;
            if (dividend >= divisor)
            {
                raw_str[str_j] = static_cast<unsigned char>(dividend / divisor);
                ++str_j;
                dividend %= divisor;
            }
            else
            {
                raw_str[str_j] = 0;
                ++str_j;
            }
        }
        // Update size.
        raw_size = str_j;
        // dividend contains remainder of the whole division.
        nout.words[nout.size] = static_cast<word_t>(dividend);
        ++nout.size;
    }
    // Copy and adjust the result.
    out.size = AP_MIN(nout.size, out.capacity);
    if (out.size != nout.size)
    {
        nout.size = out.size;
        flags.set(fregister::overflow);
    }
    // If value was with "-" sign it has to be converted to two's complement variant.
    if (nout.sign == 1)
    {
        asm_twos(rregister(nout), out);
    }
    else
    {
        asm_cp(rregister(nout), out);
    }
    asm_trim(out);
    return flags;
}

void uinteger_tstr(const rregister& in, std::string& str, index_t base, const char* digits, bool prefix)
{
    if (prefix)
    {
        str.append(prefix_for_base(base));
    }
    index_t start_index = str.size(); // Index where actual number starts, used to reverse string in the end.
    // Trivial case.
    if (in.size == 0)
    {
        str.push_back('0');
        return;
    }
    // Temporary quotient.
    array<word_t> quo_words = array_alloc<word_t>(in.size);
    wregister quo{quo_words.get(), in.size, in.size, false};
    // Temporary remainder.
    word_t rem_words[1];
    wregister rem{&rem_words[0], 1, 1, false};
    // First division.
    asm_div_short(in, base, quo, rem);
    str.push_back(digits[rem.words[0]]);
    asm_trim(quo);
    // Subsequent divisions.
    while (quo.size != 0)
    {
        asm_div_short(rregister(quo), base, quo, rem);
        str.push_back(digits[rem.words[0]]);
        asm_trim(quo);
    }
    // Swap to obtain correct order.
    index_t end_index = (str.size() - start_index) / 2;
    for (index_t i = 0; i < end_index; ++i)
    {
        std::swap(str[i + start_index], str[str.size() - i - 1]);
    }
}

fregister uinteger_fbasic(wregister& out, unsigned long long basic)
{
    fregister flags;
    out.size = 0;
    index_t i = 0;
    while ((basic != 0) && (i < out.capacity))
    {
        out.words[i] = static_cast<word_t>(basic);
        basic >>= word_traits::bits;
        ++i;
    }
    if (basic != 0)
    {
        flags.set(fregister::overflow);
    }
    out.size = i;
    asm_trim(out);
    return flags;
}

unsigned long long uinteger_tbasic(rregister in)
{
    unsigned long long result = 0;
    in.size = AP_MIN(in.size, sizeof(result) / sizeof(word_t));
    for (index_t i = 0; i < in.size; ++i)
    {
        result <<= word_traits::bits;
        result |= in.words[(in.size - 1) - i];
    }
    return result;
}

fregister uinteger_scp(rregister in, wregister& out)
{
    fregister flags;
    WRAP(in, out);
    asm_cp(in, out);
    asm_trim(out);
    return flags;
}

fregister uinteger_tos(rregister in, wregister& out)
{
    fregister flags = uinteger_scp(in, out);
    return snorm(out, flags);
}

cmpres uinteger_cmp(const rregister& left, const rregister& right)
{
    if (left.size > right.size)
    {
        return cmpres{cmpres::greater, left.size};
    }
    else if (left.size < right.size)
    {
        return cmpres{cmpres::less, right.size};
    }
    else
    {
        return asm_cmp(left, right);
    }
}

fregister uinteger_add(rregister left, rregister right, wregister& out)
{
    fregister flags;
    if (left.size < right.size)
    {
        std::swap(left, right);
    }
    WRAP(left, out);
    WRAP(right, out);
    if (asm_add(left, right, out) != 0)
    {
        flags.set(fregister::overflow);
    }
    asm_trim(out);
    return flags;
}

fregister uinteger_sub(rregister left, rregister right, wregister& out)
{
    fregister flags;
    WRAP(left, out);
    WRAP(right, out);
    cmpres res = uinteger_cmp(left, right);

    if (res.result == cmpres::greater)
    {
        left.size = res.size;
        right.size = AP_MIN(right.size, res.size);
        asm_sub(left, right, out);
    }
    else if (res.result == cmpres::less)
    {
        flags.set(fregister::overflow);
        left.size = AP_MIN(left.size, res.size);
        right.size = res.size;
        asm_sub(right, left, out);
        asm_twos(rregister(out), out);
    }
    else
    {
        out.size = 0;
    }

    asm_trim(out);
    return flags;
}

fregister uinteger_mul(rregister left, rregister right, wregister& out)
{
    fregister flags;
    // Reorder operands.
    if (left.size < right.size)
    {
        std::swap(left, right);
    }
    // Check if right is not zero (do not care about left after reorder)
    if (right.size == 0)
    {
        out.size = 0;
        return flags;
    }
    // Check for obvious overflow.
    if ((left.size + right.size) > (out.capacity + 1))
    {
        flags.set(flags.overflow);
    }

    WRAP(left, out);
    WRAP(right, out);
    // Perform actual multiplication.
    wregister nout = out;
    array<word_t> nout_words = array_null<word_t>();
    nout.size = 0;
    if ((out.words == left.words) || (out.words == right.words))
    {
        nout_words = array_alloc<word_t>(nout.capacity);
        nout.words = nout_words.get();
    }
    dword_t carry = 0;
    if (right.size == 1)
    {
        carry = asm_mul_short(left, right.words[0], nout);
    }
    else
    {
        carry = asm_mul(left, right, nout);
    }

    if (carry != 0)
    {
        flags.set(fregister::overflow);
    }
    if (nout.words != out.words)
    {
        asm_cp(rregister(nout), out);
    }
    else
    {
        out.size = nout.size;
    }
    asm_trim(out);
    return flags;
}

fregister uinteger_div(rregister left, rregister right, wregister& quo, wregister& rem)
{
    fregister flags;
    // Glorious zero division.
    if (right.size == 0)
    {
        quo.size = 0;
        rem.size = 0;
        flags.set(fregister::infinity);
        return flags;
    }
    // Setup quotient and remainder, if necessary.
    array<word_t> quo_words = array_null<word_t>();
    array<word_t> rem_words = array_null<word_t>();
    if (quo.capacity == 0)
    {
        quo.capacity = est_quo(left.size, right.size);
        quo_words = array_alloc<word_t>(quo.capacity);
        quo.words = quo_words.get();
    }
    if (rem.capacity == 0)
    {
        rem.capacity = est_rem(left.size, right.size);
        rem_words = array_alloc<word_t>(rem.capacity);
        rem.words = rem_words.get();
    }
    // If left is obviously less, return immediately.
    if (left.size < right.size)
    {
        quo.size = 0;
        flags |= uinteger_scp(left, rem);
    }
    else
    {
        // Do not wrap, bit pattern will be wrong.
        if (left.size > quo.capacity || right.size > rem.capacity)
        {
            flags |= fregister::overflow;
        }
        if (right.size == 1)
        {
            asm_div_short(left, right.words[0], quo, rem);
        }
        else
        {
            asm_div(left, right, quo, rem);
        }
    }
    if (quo_words.get() != nullptr)
    {
        quo.words = nullptr;
        asm_trim(rem);
    }
    if (rem_words.get() != nullptr)
    {
        rem.words = nullptr;
        asm_trim(quo);
    }
    return flags;
}

fregister uinteger_quo(rregister left, rregister right, wregister& out)
{
    wregister rem{nullptr, 0, 0, false};
    return uinteger_div(left, right, out, rem);
}

fregister uinteger_rem(rregister left, rregister right, wregister& out)
{
    wregister quo{nullptr, 0, 0, false};
    return uinteger_div(left, right, quo, out);
}

template <asm_bit_op op>
fregister uinteger_bit(rregister left, rregister right, wregister& out)
{
    fregister flags;
    WRAP(left, out);
    WRAP(right, out);
    if (left.size < right.size)
    {
        std::swap(left, right);
    }
    op(left, right, out);
    asm_trim(out);
    return flags;
}

fregister uinteger_and(rregister left, rregister right, wregister& out)
{
    return uinteger_bit<asm_and>(left, right, out);
}

fregister uinteger_or(rregister left, rregister right, wregister& out)
{
    return uinteger_bit<asm_or>(left, right, out);
}

fregister uinteger_xor(rregister left, rregister right, wregister& out)
{
    return uinteger_bit<asm_xor>(left, right, out);
}

fregister uinteger_not(rregister in, wregister& out)
{
    fregister flags;
    WRAP(in, out);
    asm_not(in, out);
    asm_trim(out);
    return flags;
}

fregister uinteger_rsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    if (in.size == 0)
    {
        out.size = 0;
        return flags;
    }
    if (in.size > out.capacity)
    {
        flags.set(fregister::overflow);
    }
    index_t wshift = shift / word_traits::bits;
    shift %= word_traits::bits;
    if (in.size <= wshift)
    {
        out.size = 0;
        return flags;
    }
    in.size -= wshift;
    in.words += wshift;
    in.size = AP_MIN(in.size, out.capacity + 1);
    asm_rsh(in, shift, out);
    asm_trim(out);
    return flags;
}

fregister uinteger_lsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    if (in.size == 0)
    {
        return uinteger_scp(in, out);
    }
    WRAP(in, out);
    index_t wshift = shift / word_traits::bits;
    shift %= word_traits::bits;
    if (out.capacity <= wshift)
    {
        out.size = 0;
        return flags;
    }
    // Adjusted out. Copy in order to not mess up original register.
    wregister aout = out;
    aout.capacity -= wshift;
    aout.words += wshift;
    // Updeta in.size as well.
    in.size = AP_MIN(in.size, aout.capacity);
    asm_lsh(in, shift, aout);
    // Set size on out correctly.
    out.size = wshift + aout.size;
    // Fill first wshift words with 0.
    aout.capacity = wshift;
    aout.words -= wshift;
    asm_fill(aout, 0);
    asm_trim(out);
    return flags;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// uapi definitions.

fregister sinteger_fstr(wregister& out, const char* str, index_t size, index_t base, const char* digits)
{
    // Trivial case.
    if (size == 0)
    {
        out.size = 0;
        out.sign = 0;
    }
    // Detect sign.
    out.sign = (str[0] == '-');
    if (out.sign || str[0] == '+')
    {
        ++str;
        --size;
    }
    // Convert using uinteger_fstr.
    fregister flags = uinteger_fstr(out, str, size, base, digits);
    if (out.has_msb())
    {
        flags.set(fregister::overflow);
    }
    return snorm(out, flags);
}

void sinteger_tstr(const rregister& in, std::string& str, index_t base, const char* digits, bool prefix)
{
    if (in.sign)
    {
        str.push_back('-');
    }
    uinteger_tstr(in, str, base, digits, prefix);
}

fregister sinteger_fbasic(wregister& out, signed long long basic)
{
    // Cast to unsigned, and obtain info about sign.
    unsigned long long ubasic = basic;
    if (basic < 0)
    {
        out.sign = 1;
        ubasic = (~ubasic + 1);
    }
    fregister flags = uinteger_fbasic(out, ubasic);
    if (out.has_msb())
    {
        flags.set(fregister::overflow);
    }
    return snorm(out, flags);
}

signed long long sinteger_tbasic(rregister in)
{
    unsigned long long result = uinteger_tbasic(in);
    if (in.sign)
    {
        result = ~result + 1;
    }
    return static_cast<signed long long>(result);
}

fregister sinteger_scp(rregister in, wregister& out)
{
    fregister flags = uinteger_scp(in, out);
    out.sign = in.sign;
    if (out.has_msb())
    {
        flags.set(fregister::overflow);
    }
    return snorm(out, flags);
}

fregister sinteger_tou(rregister in, wregister& out)
{
    fregister flags;
    WRAP(in, out);
    if (in.sign == 1)
    {
        asm_twos(in, out);
    }
    else
    {
        asm_cp(in, out);
    }
    out.sign = 0;
    asm_trim(out);
    return flags;
}

cmpres sinteger_cmp(const rregister& left, const rregister& right)
{
    cmpres result;
    if (left.sign != right.sign)
    {
        result.result = cmpres::greater;
    }
    else
    {
        result = uinteger_cmp(left, right);
    }
    if (left.sign)
    {
        result.result = -result.result;
    }
    return result;
}

static inline fregister sinteger_add_impl(rregister left, rregister right, wregister& out)
{
    fregister flags = uinteger_add(left, right, out);
    return snorm(out, flags);
}

static inline fregister sinteger_sub_impl(rregister left, rregister right, wregister& out)
{
    fregister flags;
    WRAP(left, out);
    WRAP(right, out);
    cmpres res = uinteger_cmp(left, right);

    if (res.result == cmpres::greater)
    {
        out.sign = left.sign;
        left.size = res.size;
        right.size = AP_MIN(right.size, res.size);
        asm_sub(left, right, out);
    }
    else if (res.result == cmpres::less)
    {
        if (out.words != right.words)
        {
            out.sign += right.sign;
        }
        left.size = AP_MIN(left.size, res.size);
        right.size = res.size;
        asm_sub(right, left, out);
    }
    else
    {
        out.size = 0;
    }

    return snorm(out, flags);
}

fregister sinteger_add(rregister left, rregister right, wregister& out)
{
    if (left.sign == right.sign)
    {
        out.sign = left.sign;
        return sinteger_add_impl(left, right, out);
    }
    else
    {
        return sinteger_sub_impl(left, right, out);
    }
}

fregister sinteger_sub(rregister left, rregister right, wregister& out)
{
    out.sign = 0;
    if (left.sign != right.sign)
    {
        out.sign = left.sign;
        return sinteger_add_impl(left, right, out);
    }
    else
    {
        out.sign = 1;
        return sinteger_sub_impl(left, right, out);
    }
}

fregister sinteger_mul(rregister left, rregister right, wregister& out)
{
    out.sign = (left.sign != right.sign);
    fregister flags = uinteger_mul(left, right, out);
    return snorm(out, flags);
}

fregister sinteger_div(rregister left, rregister right, wregister& quo, wregister& rem)
{
    fregister flags = uinteger_div(left, right, quo, rem);
    quo.sign = (left.sign != right.sign);
    rem.sign = left.sign;
    if (quo.words != nullptr)
    {
        flags |= snorm(quo, flags);
    }
    if (rem.words != nullptr)
    {
        flags |= snorm(rem, flags);
    }
    return flags;
}

fregister sinteger_quo(rregister left, rregister right, wregister& out)
{
    wregister rem{nullptr, 0, 0, false};
    return sinteger_div(left, right, out, rem);
}

fregister sinteger_rem(rregister left, rregister right, wregister& out)
{
    wregister quo{nullptr, 0, 0, false};
    return sinteger_div(left, right, quo, out);
}

template <asm_bit_op op>
fregister sinteger_bit(rregister left, rregister right, wregister& out)
{
    fregister flags;
    array<word_t> left_words = array_null<word_t>();
    array<word_t> right_words = array_null<word_t>();
    if (left.sign)
    {
        left_words = array_alloc<word_t>(out.capacity);
        wregister wuleft{left_words.get(), out.capacity, left.size, left.sign};
        sinteger_tou(left, wuleft);
        left.words = wuleft.words;
        left.size = wuleft.size;
        left.sign = 0;
    }
    if (right.sign)
    {
        right_words = array_alloc<word_t>(out.capacity);
        wregister wuright{right_words.get(), out.capacity, right.size, right.sign};
        sinteger_tou(right, wuright);
        right.words = wuright.words;
        right.size = wuright.size;
        right.sign = 0;
    }
    flags |= uinteger_bit<op>(left, right, out);
    flags = snorm(out, flags);
    return flags;
}

fregister sinteger_and(rregister left, rregister right, wregister& out)
{
    return sinteger_bit<asm_and>(left, right, out);
}

fregister sinteger_or(rregister left, rregister right, wregister& out)
{
    return sinteger_bit<asm_or>(left, right, out);
}

fregister sinteger_xor(rregister left, rregister right, wregister& out)
{
    return sinteger_bit<asm_xor>(left, right, out);
}

fregister sinteger_not(rregister in, wregister& out)
{
    fregister flags;
    array<word_t> in_words = array_null<word_t>();
    if (in.sign)
    {
        in_words = array_alloc<word_t>(out.capacity);
        wregister wuin{in_words.get(), out.capacity, in.size, in.sign};
        sinteger_tou(in, wuin);
        in.words = wuin.words;
        in.size = wuin.size;
        out.sign = 0;
    }
    flags |= uinteger_not(in, out);
    return snorm(out, flags);
}

fregister sinteger_rsh(rregister in, index_t shift, wregister& out)
{
    fregister flags;
    if ((in.size == 0) || (shift == 0))
    {
        return sinteger_scp(in, out);
    }
    index_t wshift = shift / word_traits::bits;
    shift %= word_traits::bits;
    out.sign = in.sign;
    if (in.size <= wshift)
    {
        // Either all 0, or all 1, there is no typo.
        out.size = in.sign;
        out.words[0] = in.sign;
        return flags;
    }
    in.size -= wshift;
    in.words += wshift;
    word_t rem_words[1] = {0};
    rregister rem{&rem_words[0], 1, 1, false};
    // Right shift of negative is a division by 2 with rounding "up".
    // So if at least one non-zero bit was discarded, out shall be increased by 1.
    if (in.sign)
    {
        rem_words[0] = bool(in.words[0] & word_t(word_t(word_traits::ones << (word_traits::bits - shift)) >> (word_traits::bits - shift)));
        if (rem_words[0] == 0)
        {
            in.words -= wshift;
            for (index_t i = 0; i < wshift; ++i)
            {
                if (in.words[i] != 0)
                {
                    rem_words[0] = 1;
                    break;
                }
            }
            in.words += wshift;
        }
    }
    in.size = AP_MIN(in.size, out.capacity + 1);
    asm_rsh(in, shift, out);
    asm_add(rregister(out), rem, out);
    return snorm(out, flags);
}

fregister sinteger_lsh(rregister in, index_t shift, wregister& out)
{
    out.sign = in.sign;
    fregister flags = uinteger_lsh(in, shift, out);
    return snorm(out, flags);
}

} // namespace library
} // namespace ap

#endif
