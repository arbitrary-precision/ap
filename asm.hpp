#ifndef DEOHAYER_AP_ASM_HPP
#define DEOHAYER_AP_ASM_HPP

#define FLEX_DEBUG_LEVEL 255
#include "array.hpp"
#include "core.hpp"
#include <cstring>

//
// All of the functions below represent bare algorithms. They work with unsigned multiprecision values.
// However, they put strict limitations on their operands. Each operation always yields correct two's complement
// bit pattern. Since values are unsigned, it is up to the upper layers, how this bit pattern is treated.
//

namespace ap
{
namespace library
{

struct cmpres
{
    int result;
    index_t size;

    cmpres() : cmpres(0, 0) {}
    cmpres(int _result, index_t _size) : result(_result), size(_size) {}

    cmpres(const cmpres& other) = default;
    cmpres& operator=(const cmpres& other) = default;

    cmpres(cmpres&& other) = default;
    cmpres& operator=(cmpres&& other) = default;

    enum
    {
        less = -1,
        greater = -less,
        equal = 0
    };
};

// Arithmetically compare two registers of equal size.
// Return: Result of comparison and size of the unequal part.
// Change: None.
// Preconditions:
// left.size == right.size
ap_linkage cmpres asm_cmp(const rregister& left, const rregister& right);

// Adjust size to ensure that words[0, size) contains no leading zeros.
// Return: Adjusted size.
// Change: None.
// Preconditions: None
ap_linkage index_t asm_trim(const word_t* const words, index_t size);

// Adjust inout.size to ensure that inout.words[0, inout.size) contains no leading zeros.
// Return: None.
// Change:
// inout.size adjusted accordingly.
// Preconditions: None.
template <typename _Words>
void asm_trim(dregister<_Words>& inout)
{
    inout.size = asm_trim(inout.words, inout.size);
}

// Fill inout with word to full capacity.
// Return: None.
// Change:
// inout.size = inout.capacity.
// inout.words is filled with word.
// Preconditions: None.
ap_linkage void asm_fill(wregister& inout, word_t word);

// Copy words from in to out.
// Return: None.
// Change:
// out.size = in.size.
// out.words[0, out.size) is word-wise equal to in.words[0, in.size).
// Preconditions:
// in.size in [0, out.capacity].
ap_linkage void asm_cp(const rregister& in, wregister& out);

// Write two's complement of in to out.
// Return: None.
// Change:
// out.size = out.capacity.
// out.words[0, out.capacity) contains untrimmed two's complement of in.
// Preconditions:
// in.size in [0, out.capacity].
ap_linkage void asm_twos(const rregister& in, wregister& out);

// Word-wise wrapping addition of left and right.
// Return: Carry (1 if overflow, 0 otherwise).
// Change:
// out.size = AP_MIN(left.size + 1, out.capacity).
// out.words contains untrimmed wrapped two's complement pattern of the result of operation.
// Preconditions:
// right.size in [0, left.size].
// left.size in [0, out.capacity].
ap_linkage dword_t asm_add(const rregister& left, const rregister& right, wregister& out);

// Word-wise subtraction of right from left.
// Return: None.
// Change:
// out.size = left.size.
// out.words contains untrimmed two's complement pattern of the result of operation.
// Preconditions:
// right is arithmetically less than left.
// left.size in [0, out.capacity].
ap_linkage void asm_sub(const rregister& left, const rregister& right, wregister& out);

// Word-wise wrapping long-short multiplication left by right with addition to out (long multiplication step).
// Return: Carry of the operation, if overflow occured.
// Change:
// out.size = AP_MIN(left.size + 1, out.capacity).
// out.words contains untrimmed wrapped two's complement pattern of (left * right + out).
// Preconditions:
// right in [0, word_traits::ones].
// out.size in [0, left.size].
// left.size in [0, out.capacity].
ap_linkage dword_t asm_mul_short(const rregister& left, dword_t right, wregister& out);

// Word-wise wrapping long-long multiplication left by right.
// Return: Carry of the operation, if overflow occured.
// Case of an obvious overflow shall be tracked by the caller (left.size + right.size > out.capacity + 1).
// Change:
// out.size = AP_MIN(left.size + right.size, out.capacity).
// out.words contains untrimmed wrapped two's complement pattern of (left * right).
// Preconditions:
// Trimmed operands.
// left.words != out.words
// right.words != out.words
// right.size in [1, left.size].
// left.size in [1, out.capacity].
ap_linkage dword_t asm_mul(const rregister& left, const rregister& right, wregister& out);

// Long-short division left by right.
// Return: None.
// Change:
// quo.size = left.size.
// quo.words contains untrimmed two's complement pattern of (left / right).
// rem.size = 1.
// rem.words contains untrimmed two's complement pattern of (left % right).
// Preconditions:
// right in [1, word_traits::ones].
// left.size in [1, out.capacity].
ap_linkage void asm_div_short(const rregister& left, dword_t right, wregister& quo, wregister& rem);

// Long-long division left by right.
// Return: None.
// Change:
// quo.size = left.size + 1 - right.size.
// quo.words contains untrimmed two's complement pattern of (left / right).
// rem.size = right.size.
// rem.words contains untrimmed two's complement pattern of (left % right).
// Preconditions:
// rem.capacity >= right.size.
// right.size in [2, left.size].
// left.size in [2, quo.capacity].
ap_linkage void asm_div(const rregister& left, const rregister& right, wregister& quo, wregister& rem);

// Word-wise OR operation.
// Return: None.
// Change:
// out.size = left.size.
// out.words contains untrimmed two's complement pattern of (left | right).
// Preconditions:
// right.size in [0, left.size].
// left.size in [0, out.capacity].
ap_linkage void asm_or(const rregister& left, const rregister& right, wregister& out);

// Word-wise XOR operation.
// Return: None.
// Change:
// out.size = left.size.
// out.words contains untrimmed two's complement pattern of (left ^ right).
// Preconditions:
// right.size in [0, left.size].
// left.size in [0, out.capacity].
ap_linkage void asm_xor(const rregister& left, const rregister& right, wregister& out);

// Word-wise AND operation.
// Return: None.
// Change:
// out.size = left.size.
// out.words contains untrimmed two's complement pattern of (left & right).
// Preconditions:
// right.size in [0, left.size].
// left.size in [0, out.capacity].
ap_linkage void asm_and(const rregister& left, const rregister& right, wregister& out);

// Word-wise NOT operation.
// Return: None.
// Change:
// out.size = out.capacity.
// out.words contains untrimmed two's complement pattern of ~in, with extra words filled as all-ones.
// Preconditions:
// in.size in [0, out.capacity].
ap_linkage void asm_not(const rregister& in, wregister& out);

// Word-wise right shift.
// Return: None.
// Change:
// out.size = in.size.
// out.words contains untrimmed two's complement pattern of (in >> shift).
// Preconditions:
// shift in [1, word_traits::bits).
// in.size in [1, out.capacity].
ap_linkage void asm_rsh(const rregister& in, index_t shift, wregister& out);

// Word-wise left shift.
// Return: None.
// Change:
// out.size = AP_MIN(in.size + 1, out.capacity).
// out.words contains untrimmed wrapped two's complement pattern of (in << shift).
// Preconditions:
// shift in [1, word_traits::bits).
// in.size in [1, out.capacity].
ap_linkage void asm_lsh(const rregister& in, index_t shift, wregister& out);

} // namespace library
} // namespace ap

#ifndef AP_USE_SOURCES
#define DEOHAYER_AP_ASM_CPP 0
#include "asm.cpp"
#endif

#endif