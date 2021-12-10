#ifndef DEOHAYER_AP_ASM_CPP
#define DEOHAYER_AP_ASM_CPP 1
#endif
#if (DEOHAYER_AP_ASM_CPP == 1) == defined(AP_USE_SOURCES)

#include "asm.hpp"

namespace ap
{
namespace library
{

cmpres asm_cmp(const rregister& left, const rregister& right)
{
    for (index_t i = left.size; i > 0;)
    {
        --i;
        if (left.words[i] > right.words[i])
        {
            return cmpres{cmpres::greater, i + 1};
        }
        else if (left.words[i] < right.words[i])
        {
            return cmpres{cmpres::less, i + 1};
        }
    }
    return cmpres{cmpres::equal, 0};
}

index_t asm_trim(const word_t* const words, index_t size)
{
    const word_t* const _words = words - 1;
    for (index_t i = size; i > 0; --i)
    {
        if (_words[i] != 0)
        {
            return i;
        }
    }
    return 0;
}

void asm_fill(wregister& inout, word_t word)
{
    inout.size = inout.capacity;
    for (index_t i = 0; i < inout.size; ++i)
    {
        inout.words[i] = word;
    }
}

void asm_cp(const rregister& in, wregister& out)
{
    out.size = in.size;
    std::memcpy(out.words, in.words, in.size * word_traits::bytes);
}

void asm_twos(const rregister& in, wregister& out)
{
    out.size = out.capacity;
    dword_t carry = 1;
    index_t i = 0;
    for (; i < in.size; ++i)
    {
        carry += static_cast<word_t>(~in.words[i]);
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }
    for (; i < out.capacity; ++i)
    {
        carry += word_traits::ones;
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }
}

dword_t asm_add(const rregister& left, const rregister& right, wregister& out)
{
    dword_t carry = 0;
    index_t i = 0;

    for (; i < right.size; ++i)
    {
        carry += left.words[i];
        carry += right.words[i];
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }

    for (; i < left.size; ++i)
    {
        carry += left.words[i];
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }

    if (out.capacity > left.size)
    {
        out.words[i] = static_cast<word_t>(carry);
        ++i;
        if (carry != 0)
        {
            carry = 0;
        }
    }

    out.size = i;
    return carry;
}

void asm_sub(const rregister& left, const rregister& right, wregister& out)
{
    static constexpr index_t borrow_shift = dword_traits::bits - 1;
    dword_t borrow = 0;
    index_t i = 0;

    for (; i < right.size; ++i)
    {
        borrow = left.words[i] - borrow;
        borrow -= right.words[i];
        out.words[i] = static_cast<word_t>(borrow);
        borrow >>= borrow_shift;
    }

    for (; i < left.size; ++i)
    {
        borrow = left.words[i] - borrow;
        out.words[i] = static_cast<word_t>(borrow);
        borrow >>= borrow_shift;
    }

    out.size = i;
}

dword_t asm_mul_short(const rregister& left, dword_t right, wregister& out)
{
    const index_t stop_index = AP_MIN(left.size, out.capacity);
    dword_t carry = 0;
    index_t i = 0;

    for (; i < out.size; ++i)
    {
        carry += out.words[i];
        carry += right * left.words[i];
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }

    for (; i < stop_index; ++i)
    {
        carry += right * left.words[i];
        out.words[i] = static_cast<word_t>(carry);
        carry >>= word_traits::bits;
    }

    if (out.capacity > stop_index)
    {
        out.words[i] = static_cast<word_t>(carry);
        ++i;
        if (carry != 0)
        {
            carry = 0;
        }
    }

    out.size = i;
    return carry;
}

dword_t asm_mul(const rregister& left, const rregister& right, wregister& out)
{
    wregister out_tmp = out;
    dword_t carry = 0;
    out_tmp.size = 0;
    for (index_t i = 0; i < right.size; ++i)
    {
        carry += asm_mul_short(left, right.words[i], out_tmp);
        ++out_tmp.words;
        --out_tmp.capacity;
        --out_tmp.size;
    }
    out.size = out_tmp.size + right.size;
    return carry;
}

void asm_div_short(const rregister& left, dword_t right, wregister& quo, wregister& rem)
{
    index_t i = left.size - 1;

    dword_t carry = left.words[i] % right;
    if (i < quo.capacity)
    {
        quo.words[i] = static_cast<word_t>(left.words[i] / right);
    }
    quo.size = AP_MIN(left.size, quo.capacity);

    while (i > 0)
    {
        --i;
        carry <<= word_traits::bits;
        carry |= left.words[i];
        if (i < quo.capacity)
        {
            quo.words[i] = static_cast<word_t>(carry / right);
        }
        carry %= right;
    }
    rem.words[0] = static_cast<word_t>(carry);
    rem.size = 1;
}

void asm_div(const rregister& left, const rregister& right, wregister& quo, wregister& rem)
{
    static constexpr index_t borrow_shift = dword_traits::bits - 1;
    static constexpr dword_t base = dword_t{word_traits::ones} + 1;
    const dword_t normalizer = ((base / 2) / right.words[right.size - 1]) + ((base / 2) % right.words[right.size - 1] != 0);

    // Normalize dividend (left).
    auto nleft_digits = array_alloc<word_t>(left.size + 1);
    wregister nleft{nleft_digits.get(), index_t(left.size + 1), 0, false};
    asm_mul_short(left, normalizer, nleft);
    dword_t u2 = 0;
    dword_t u1 = 0;
    dword_t u0 = 0;

    // Normalize divisor (right).
    auto nright_digits = array_alloc<word_t>(right.size);
    wregister nright{nright_digits.get(), right.size, 0, false};
    asm_mul_short(right, normalizer, nright);
    const dword_t v1 = nright.words[nright.size - 1];
    const dword_t v0 = nright.words[nright.size - 2];

    // Single division step context.

    // Difference in size of normalized operands. "+ 1" because u2 acts as extra most significant word of rem.
    const index_t size_diff = (nleft.size - nright.size);
    index_t j = 0;      // Number of single-word divisions.
    index_t i = 0;      // Iterator for single-word divisions.
    dword_t q = 0;      // Single-word quotient, written to rem at the end of iteration.
    dword_t r = 0;      // Single word remainder, used in q approximation.
    dword_t carry = 0;  // Used in multiply-subtract part of single-word division.
    dword_t borrow = 0; // Used in multiply-subtract part of single-word division.

    for (j = size_diff; j > 0;)
    {
        --j;
        i = j + nright.size;
        u2 = nleft.words[i];
        u1 = nleft.words[i - 1];
        u0 = nleft.words[i - 2];
        q = u2;
        q <<= word_traits::bits;
        q += u1;
        r = q % v1;
        q /= v1;
        if (q == base || q * v0 > (base * r + u0))
        {
            --q;
            r += v1;
            if (r < base)
            {
                if (q == base || q * v0 > (base * r + u0))
                {
                    --q;
                    r += v1;
                }
            }
        }
        i = j;
        borrow = 0;
        carry = 0;
        for (index_t k = 0; k < right.size; ++k, ++i)
        {
            borrow = nleft.words[i] - borrow;
            carry += nright.words[k] * q;
            borrow -= static_cast<word_t>(carry);
            nleft.words[i] = static_cast<word_t>(borrow);
            borrow >>= borrow_shift;
            carry >>= (word_traits::bits);
        }
        borrow += carry;
        if (borrow != 0)
        {
            if (nleft.words[i] >= borrow)
            {
                nleft.words[i] -= static_cast<word_t>(borrow);
            }
            else // unlikely
            {
                --q;
                i = j;
                carry = 0;
                for (size_t k = 0; k < right.size; ++k, ++i)
                {
                    carry += nleft.words[i];
                    carry += nright.words[k];
                    nleft.words[i] = static_cast<word_t>(carry);
                    carry >>= word_traits::bits;
                }
                nleft.words[i] += static_cast<word_t>(carry);
                nleft.words[i] -= static_cast<word_t>(borrow);
            }
        }
        if (j < quo.capacity)
        {
            quo.words[j] = static_cast<word_t>(q);
        }
    }
    quo.size = AP_MIN(size_diff, quo.capacity);
    nleft.size = i;
    if (nleft.size != 0)
    {
        asm_div_short(rregister(nleft), normalizer, rem, nright);
    }
}

using bit_op_t = word_t (*)(word_t, word_t);

template <bit_op_t op>
void asm_bit(const rregister& left, const rregister& right, wregister& out)
{
    index_t i = 0;
    for (; i < right.size; ++i)
    {
        out.words[i] = op(left.words[i], right.words[i]);
    }
    for (; i < left.size; ++i)
    {
        out.words[i] = op(left.words[i], 0);
    }
    out.size = left.size;
}

static inline word_t bit_or(word_t a, word_t b)
{
    return a | b;
}

void asm_or(const rregister& left, const rregister& right, wregister& out)
{
    asm_bit<bit_or>(left, right, out);
}

static inline word_t bit_xor(word_t a, word_t b)
{
    return a ^ b;
}

void asm_xor(const rregister& left, const rregister& right, wregister& out)
{
    asm_bit<bit_xor>(left, right, out);
}

static inline word_t bit_and(word_t a, word_t b)
{
    return a & b;
}

void asm_and(const rregister& left, const rregister& right, wregister& out)
{
    asm_bit<bit_and>(left, right, out);
}

void asm_not(const rregister& in, wregister& out)
{
    for (index_t i = 0; i < in.size; ++i)
    {
        out.words[i] = ~in.words[i];
    }
    for (index_t i = in.size; i < out.capacity; ++i)
    {
        out.words[i] = word_traits::ones;
    }
    out.size = out.capacity;
}

void asm_rsh(const rregister& in, index_t shift, wregister& out)
{
    dword_t dword = in.words[0];
    for (index_t i = 1; i < in.size; ++i)
    {
        dword |= (dword_t{in.words[i]} << word_traits::bits);
        out.words[i - 1] = static_cast<word_t>(dword >> shift);
        dword >>= word_traits::bits;
    }
    if (out.capacity >= in.size)
    {
        out.words[in.size - 1] = static_cast<word_t>(dword >> shift);
        out.size = in.size;
    }
    else
    {
        out.size = in.size - 1;
    }
}

void asm_lsh(const rregister& in, index_t shift, wregister& out)
{
    // First word will be overwritten if in and out refer to the same number.
    word_t first = (in.words[0] << shift);
    dword_t dword = in.words[0];
    shift = word_traits::bits - shift;
    for (index_t i = 1; i < in.size; ++i)
    {
        dword |= (dword_t{in.words[i]} << word_traits::bits);
        out.words[i] = static_cast<word_t>(dword >> shift);
        dword >>= word_traits::bits;
    }
    out.words[0] = first;
    out.size = in.size;
    if (out.capacity > in.size)
    {
        out.words[in.size] = static_cast<word_t>(dword >> shift);
        ++out.size;
    }
}

} // namespace library
} // namespace ap

#endif